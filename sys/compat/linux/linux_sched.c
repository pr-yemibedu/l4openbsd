/*	$OpenBSD: linux_sched.c,v 1.8 2010/01/04 19:27:21 guenther Exp $	*/
/*	$NetBSD: linux_sched.c,v 1.6 2000/05/28 05:49:05 thorpej Exp $	*/

/*-
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center; by Matthias Scheler.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Linux compatibility module. Try to deal with scheduler related syscalls.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/syscallargs.h>

#include <machine/cpu.h>

#include <compat/linux/linux_types.h>
#include <compat/linux/linux_sched.h>
#include <compat/linux/linux_signal.h>
#include <compat/linux/linux_syscallargs.h>

int
linux_sys_clone(p, v, retval)
	struct proc *p;
	void *v;
	register_t *retval;
{
	struct linux_sys_clone_args /* {
		syscallarg(int) flags;
		syscallarg(void *) stack;
	} */ *uap = v;
	int cflags = SCARG(uap, flags);
	int flags = FORK_RFORK, sig;

	/*
	 * We only support certain bits.  The Linux crew keep adding more,
	 * so let's test for anything outside of what we support and complain
	 * about them.  Not everything in this list is completely supported,
	 * they just aren't _always_ an error.
	 * To make nptl threads work we need to add support for at least
	 * CLONE_SETTLS, CLONE_PARENT_SETTID, and CLONE_CHILD_CLEARTID.
	 */
	if (cflags & ~(LINUX_CLONE_CSIGNAL | LINUX_CLONE_VM | LINUX_CLONE_FS |
	    LINUX_CLONE_FILES | LINUX_CLONE_SIGHAND | LINUX_CLONE_VFORK |
	    LINUX_CLONE_PARENT | LINUX_CLONE_THREAD | LINUX_CLONE_SYSVSEM |
	    LINUX_CLONE_DETACHED | LINUX_CLONE_UNTRACED))
		return (EINVAL);

	if (cflags & LINUX_CLONE_VM)
		flags |= FORK_SHAREVM;
	if (cflags & LINUX_CLONE_FILES)
		flags |= FORK_SHAREFILES;
	if (cflags & LINUX_CLONE_SIGHAND) {
		/* According to Linux, CLONE_SIGHAND requires CLONE_VM */
		if ((cflags & LINUX_CLONE_VM) == 0)
			return (EINVAL);
		flags |= FORK_SIGHAND;
	}
	if (cflags & LINUX_CLONE_VFORK)
		flags |= FORK_PPWAIT;
	if (cflags & LINUX_CLONE_THREAD) {
		/*
		 * Linux agrees with us: CLONE_THREAD requires
		 * CLONE_SIGHAND.  Unlike Linux, we also also require
		 * CLONE_FS and CLONE_SYSVSEM.  Also, we decree it
		 * to be incompatible with CLONE_VFORK, as I don't
		 * want to work out whether that's 100% safe.
		 */
#define REQUIRED	\
	(LINUX_CLONE_SIGHAND | LINUX_CLONE_FS | LINUX_CLONE_SYSVSEM)
#define BANNED		\
	LINUX_CLONE_VFORK
		if ((cflags & (REQUIRED | BANNED)) != REQUIRED)
			return (EINVAL);
		/*
		 * Linux says that CLONE_THREAD means no signal
		 * will be sent on exit (even if a non-standard
		 * signal is requested via CLONE_CSIGNAL), so pass
		 * FORK_NOZOMBIE too.
		 */
		flags |= FORK_THREAD | FORK_NOZOMBIE;
	} else {
		/*
		 * These are only supported with CLONE_THREAD.  Arguably,
		 * CLONE_FS should be in this list, because we don't
		 * support sharing of working directory and root directory
		 * (chdir + chroot) except via threads.  On the other
		 * hand, we tie the sharing of umask to the sharing of
		 * files, so a process that doesn't request CLONE_FS but
		 * does ask for CLONE_FILES is going to get some of the
		 * former's effect.  Some programs (e.g., Opera) at least
		 * _seem_ to work if we let it through, so we'll just
		 * cross our fingers for now and silently ignore it if
		 * CLONE_FILES was also requested.
		 */
		if (cflags & (LINUX_CLONE_PARENT | LINUX_CLONE_SYSVSEM))
			return (EINVAL);
		if ((cflags & (LINUX_CLONE_FS | LINUX_CLONE_FILES)) ==
		    LINUX_CLONE_FS)
			return (EINVAL);
	}
	/*
	 * Since we don't support CLONE_PTRACE, the CLONE_UNTRACED
	 * flag can be silently ignored.  CLONE_DETACHED is always
	 * ignored by Linux.
	 */

	sig = cflags & LINUX_CLONE_CSIGNAL;
	if (sig < 0 || sig >= LINUX__NSIG)
		return (EINVAL);
	sig = linux_to_bsd_sig[sig];

	/*
	 * Note that Linux does not provide a portable way of specifying
	 * the stack area; the caller must know if the stack grows up
	 * or down.  So, we pass a stack size of 0, so that the code
	 * that makes this adjustment is a noop.
	 */
	return (fork1(p, sig, flags, SCARG(uap, stack), 0, NULL, NULL, retval,
	    NULL));
}

int
linux_sys_sched_setparam(cp, v, retval)
	struct proc *cp;
	void *v;
	register_t *retval;
{
	struct linux_sys_sched_setparam_args /* {
		syscallarg(linux_pid_t) pid;
		syscallarg(const struct linux_sched_param *) sp;
	} */ *uap = v;
	int error;
	struct linux_sched_param lp;
	struct proc *p;

	/*
	 * We only check for valid parameters and return afterwards.
	 */

	if (SCARG(uap, pid) < 0 || SCARG(uap, sp) == NULL)
		return (EINVAL);

	error = copyin(SCARG(uap, sp), &lp, sizeof(lp));
	if (error)
		return (error);

	if (SCARG(uap, pid) != 0) {
		struct pcred *pc = cp->p_cred;

		if ((p = pfind(SCARG(uap, pid))) == NULL)
			return (ESRCH);
		if (!(cp == p ||
		      pc->pc_ucred->cr_uid == 0 ||
		      pc->p_ruid == p->p_cred->p_ruid ||
		      pc->pc_ucred->cr_uid == p->p_cred->p_ruid ||
		      pc->p_ruid == p->p_ucred->cr_uid ||
		      pc->pc_ucred->cr_uid == p->p_ucred->cr_uid))
			return (EPERM);
	}

	return (0);
}

int
linux_sys_sched_getparam(cp, v, retval)
	struct proc *cp;
	void *v;
	register_t *retval;
{
	struct linux_sys_sched_getparam_args /* {
		syscallarg(linux_pid_t) pid;
		syscallarg(struct linux_sched_param *) sp;
	} */ *uap = v;
	struct proc *p;
	struct linux_sched_param lp;

	/*
	 * We only check for valid parameters and return a dummy priority
	 * afterwards.
	 */
	if (SCARG(uap, pid) < 0 || SCARG(uap, sp) == NULL)
		return (EINVAL);

	if (SCARG(uap, pid) != 0) {
		struct pcred *pc = cp->p_cred;

		if ((p = pfind(SCARG(uap, pid))) == NULL)
			return (ESRCH);
		if (!(cp == p ||
		      pc->pc_ucred->cr_uid == 0 ||
		      pc->p_ruid == p->p_cred->p_ruid ||
		      pc->pc_ucred->cr_uid == p->p_cred->p_ruid ||
		      pc->p_ruid == p->p_ucred->cr_uid ||
		      pc->pc_ucred->cr_uid == p->p_ucred->cr_uid))
			return (EPERM);
	}

	lp.sched_priority = 0;
	return (copyout(&lp, SCARG(uap, sp), sizeof lp));
}

int
linux_sys_sched_setscheduler(cp, v, retval)
	struct proc *cp;
	void *v;
	register_t *retval;
{
	struct linux_sys_sched_setscheduler_args /* {
		syscallarg(linux_pid_t) pid;
		syscallarg(int) policy;
		syscallarg(cont struct linux_sched_scheduler *) sp;
	} */ *uap = v;
	int error;
	struct linux_sched_param lp;
	struct proc *p;

	/*
	 * We only check for valid parameters and return afterwards.
	 */

	if (SCARG(uap, pid) < 0 || SCARG(uap, sp) == NULL)
		return (EINVAL);

	error = copyin(SCARG(uap, sp), &lp, sizeof(lp));
	if (error)
		return (error);

	if (SCARG(uap, pid) != 0) {
		struct pcred *pc = cp->p_cred;

		if ((p = pfind(SCARG(uap, pid))) == NULL)
			return (ESRCH);
		if (!(cp == p ||
		      pc->pc_ucred->cr_uid == 0 ||
		      pc->p_ruid == p->p_cred->p_ruid ||
		      pc->pc_ucred->cr_uid == p->p_cred->p_ruid ||
		      pc->p_ruid == p->p_ucred->cr_uid ||
		      pc->pc_ucred->cr_uid == p->p_ucred->cr_uid))
			return (EPERM);
	}

	/*
	 * We can't emulate anything but the default scheduling policy.
	 */
	if (SCARG(uap, policy) != LINUX_SCHED_OTHER || lp.sched_priority != 0)
		return (EINVAL);

	return (0);
}

int
linux_sys_sched_getscheduler(cp, v, retval)
	struct proc *cp;
	void *v;
	register_t *retval;
{
	struct linux_sys_sched_getscheduler_args /* {
		syscallarg(linux_pid_t) pid;
	} */ *uap = v;
	struct proc *p;

	*retval = -1;

	/*
	 * We only check for valid parameters and return afterwards.
	 */

	if (SCARG(uap, pid) != 0) {
		struct pcred *pc = cp->p_cred;

		if ((p = pfind(SCARG(uap, pid))) == NULL)
			return (ESRCH);
		if (!(cp == p ||
		      pc->pc_ucred->cr_uid == 0 ||
		      pc->p_ruid == p->p_cred->p_ruid ||
		      pc->pc_ucred->cr_uid == p->p_cred->p_ruid ||
		      pc->p_ruid == p->p_ucred->cr_uid ||
		      pc->pc_ucred->cr_uid == p->p_ucred->cr_uid))
			return (EPERM);
	}

	/*
	 * We can't emulate anything but the default scheduling policy.
	 */
	*retval = LINUX_SCHED_OTHER;
	return (0);
}

int
linux_sys_sched_yield(cp, v, retval)
	struct proc *cp;
	void *v;
	register_t *retval;
{
	need_resched(curcpu());
	return (0);
}

int
linux_sys_sched_get_priority_max(cp, v, retval)
	struct proc *cp;
	void *v;
	register_t *retval;
{
	struct linux_sys_sched_get_priority_max_args /* {
		syscallarg(int) policy;
	} */ *uap = v;

	/*
	 * We can't emulate anything but the default scheduling policy.
	 */
	if (SCARG(uap, policy) != LINUX_SCHED_OTHER) {
		*retval = -1;
		return (EINVAL);
	}

	*retval = 0;
	return (0);
}

int
linux_sys_sched_get_priority_min(cp, v, retval)
	struct proc *cp;
	void *v;
	register_t *retval;
{
	struct linux_sys_sched_get_priority_min_args /* {
		syscallarg(int) policy;
	} */ *uap = v;

	/*
	 * We can't emulate anything but the default scheduling policy.
	 */
	if (SCARG(uap, policy) != LINUX_SCHED_OTHER) {
		*retval = -1;
		return (EINVAL);
	}

	*retval = 0;
	return (0);
}
