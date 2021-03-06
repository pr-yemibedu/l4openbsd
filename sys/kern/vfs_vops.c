/*	$OpenBSD: vfs_vops.c,v 1.2 2010/09/08 10:50:27 thib Exp $	*/
/*
 * Copyright (c) 2010 Thordur I. Bjornsson <thib@openbsd.org> 
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/vnode.h>

#ifdef VFSDEBUG
#define ASSERT_VP_ISLOCKED(vp) do {                             \
        if (((vp)->v_flag & VLOCKSWORK) && !VOP_ISLOCKED(vp))   \
                VOP_PRINT(vp);                                  \
                panic("vp not locked");                         \
} while (0)
#else
#define ASSERT_VP_ISLOCKED(vp)  /* nothing */
#endif

int
VOP_ISLOCKED(struct vnode *vp)
{
	struct vop_islocked_args a;
	a.a_vp = vp;

	if (vp->v_op->vop_islocked == NULL)
		return ((vp->v_op->vop_default)(&a));

	return ((vp->v_op->vop_islocked)(&a));
}

int
VOP_LOOKUP(struct vnode *dvp, struct vnode **vpp, 
    struct componentname *cnp)
{
	struct vop_lookup_args a;
	a.a_dvp = dvp;
	a.a_vpp = vpp;
	a.a_cnp = cnp;

	if (dvp->v_op->vop_lookup == NULL)
		return ((dvp->v_op->vop_default)(&a));

	return ((dvp->v_op->vop_lookup)(&a));
}

int
VOP_CREATE(struct vnode *dvp, struct vnode **vpp, 
    struct componentname *cnp, struct vattr *vap)
{
	struct vop_create_args a;
	a.a_dvp = dvp;
	a.a_vpp = vpp;
	a.a_cnp = cnp;
	a.a_vap = vap;

	ASSERT_VP_ISLOCKED(dvp);

	if (dvp->v_op->vop_create == NULL)
		return ((dvp->v_op->vop_default)(&a));
	return ((dvp->v_op->vop_create)(&a));
}

int
VOP_MKNOD(struct vnode *dvp, struct vnode **vpp, 
    struct componentname *cnp, struct vattr *vap)
{
	struct vop_mknod_args a;
	a.a_dvp = dvp;
	a.a_vpp = vpp;
	a.a_cnp = cnp;
	a.a_vap = vap;

	ASSERT_VP_ISLOCKED(dvp);

	if (dvp->v_op->vop_mknod == NULL)
		return ((dvp->v_op->vop_default)(&a));
	return ((dvp->v_op->vop_mknod)(&a));
}

int
VOP_OPEN(struct vnode *vp, int mode, struct ucred *cred, struct proc *p)
{
	struct vop_open_args a;
	a.a_vp = vp;
	a.a_mode = mode;
	a.a_cred = cred;
	a.a_p = p;

	if (vp->v_op->vop_open == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_open)(&a));
}

int
VOP_CLOSE(struct vnode *vp, int fflag, struct ucred *cred, struct proc *p)
{
	struct vop_close_args a;
	a.a_vp = vp;
	a.a_fflag = fflag;
	a.a_cred = cred;
	a.a_p = p;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_close == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_close)(&a));
}

int
VOP_ACCESS(struct vnode *vp, int mode, struct ucred *cred, struct proc *p)
{
	struct vop_access_args a;
	a.a_vp = vp;
	a.a_mode = mode;
	a.a_cred = cred;
	a.a_p = p;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_access == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_access)(&a));
}

int
VOP_GETATTR(struct vnode *vp, struct vattr *vap, struct ucred *cred, 
    struct proc *p)
{
	struct vop_getattr_args a;
	a.a_vp = vp;
	a.a_vap = vap;
	a.a_cred = cred;
	a.a_p = p;

	if (vp->v_op->vop_getattr == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_getattr)(&a));
}

int
VOP_SETATTR(struct vnode *vp, struct vattr *vap, struct ucred *cred, 
    struct proc *p)
{
	struct vop_setattr_args a;
	a.a_vp = vp;
	a.a_vap = vap;
	a.a_cred = cred;
	a.a_p = p;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_setattr == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_setattr)(&a));
}

int
VOP_READ(struct vnode *vp, struct uio *uio, int ioflag, struct ucred *cred)
{
	struct vop_read_args a;
	a.a_vp = vp;
	a.a_uio = uio;
	a.a_ioflag = ioflag;
	a.a_cred = cred;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_read == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_read)(&a));
}

int
VOP_WRITE(struct vnode *vp, struct uio *uio, int ioflag, 
    struct ucred *cred)
{
	struct vop_write_args a;
	a.a_vp = vp;
	a.a_uio = uio;
	a.a_ioflag = ioflag;
	a.a_cred = cred;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_write == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_write)(&a));
}

int
VOP_IOCTL(struct vnode *vp, u_long command, void *data, int fflag, 
    struct ucred *cred, struct proc *p)
{
	struct vop_ioctl_args a;
	a.a_vp = vp;
	a.a_command = command;
	a.a_data = data;
	a.a_fflag = fflag;
	a.a_cred = cred;
	a.a_p = p;

	if (vp->v_op->vop_ioctl == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_ioctl)(&a));
}

int
VOP_POLL(struct vnode *vp, int events, struct proc *p)
{
	struct vop_poll_args a;
	a.a_vp = vp;
	a.a_events = events;
	a.a_p = p;

	if (vp->v_op->vop_poll == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_poll)(&a));
}

int
VOP_KQFILTER(struct vnode *vp, struct knote *kn)
{
	struct vop_kqfilter_args a;
	a.a_vp = vp;
	a.a_kn = kn;

	if (vp->v_op->vop_kqfilter == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_kqfilter)(&a));
}

int
VOP_REVOKE(struct vnode *vp, int flags)
{
	struct vop_revoke_args a;
	a.a_vp = vp;
	a.a_flags = flags;

	if (vp->v_op->vop_revoke == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_revoke)(&a));
}

int
VOP_FSYNC(struct vnode *vp, struct ucred *cred, int waitfor, 
    struct proc *p)
{
	struct vop_fsync_args a;
	a.a_vp = vp;
	a.a_cred = cred;
	a.a_waitfor = waitfor;
	a.a_p = p;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_fsync == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_fsync)(&a));
}

int
VOP_REMOVE(struct vnode *dvp, struct vnode *vp, struct componentname *cnp)
{
	struct vop_remove_args a;
	a.a_dvp = dvp;
        a.a_vp = vp;
	a.a_cnp = cnp;

	ASSERT_VP_ISLOCKED(dvp);
	ASSERT_VP_ISLOCKED(vp);

	if (dvp->v_op->vop_remove == NULL)
		return ((dvp->v_op->vop_default)(&a));
	return ((dvp->v_op->vop_remove)(&a));
}

int
VOP_LINK(struct vnode *dvp, struct vnode *vp, struct componentname *cnp)
{
	struct vop_link_args a;
	a.a_dvp = dvp;
	a.a_vp = vp;
	a.a_cnp = cnp;

	ASSERT_VP_ISLOCKED(dvp);

	if (dvp->v_op->vop_link == NULL)
		return ((dvp->v_op->vop_default)(&a));
	return ((dvp->v_op->vop_link)(&a));
}

int
VOP_RENAME(struct vnode *fdvp, struct vnode *fvp, 
    struct componentname *fcnp, struct vnode *tdvp, struct vnode *tvp, 
    struct componentname *tcnp)
{
	struct vop_rename_args a;
	a.a_fdvp = fdvp;
	a.a_fvp = fvp;
	a.a_fcnp = fcnp;
	a.a_tdvp = tdvp;
	a.a_tvp = tvp;
	a.a_tcnp = tcnp;

	ASSERT_VP_ISLOCKED(tdvp);

	if (fdvp->v_op->vop_rename == NULL) 
		return ((fdvp->v_op->vop_default)(&a));
	return ((fdvp->v_op->vop_rename)(&a));
}

int
VOP_MKDIR(struct vnode *dvp, struct vnode **vpp, 
    struct componentname *cnp, struct vattr *vap)
{
	struct vop_mkdir_args a;
	a.a_dvp = dvp;
	a.a_vpp = vpp;
	a.a_cnp = cnp;
	a.a_vap = vap;

	ASSERT_VP_ISLOCKED(dvp);

	if (dvp->v_op->vop_mkdir == NULL)
		return ((dvp->v_op->vop_default)(&a));
	return ((dvp->v_op->vop_mkdir)(&a));
}

int
VOP_RMDIR(struct vnode *dvp, struct vnode *vp, struct componentname *cnp)
{
	struct vop_rmdir_args a;
	a.a_dvp = dvp;
	a.a_vp = vp;
	a.a_cnp = cnp;

	ASSERT_VP_ISLOCKED(dvp);
	ASSERT_VP_ISLOCKED(vp);

	if (dvp->v_op->vop_rmdir == NULL)
		return ((dvp->v_op->vop_default)(&a));
	return ((dvp->v_op->vop_rmdir)(&a));
}

int
VOP_SYMLINK(struct vnode *dvp, struct vnode **vpp, 
    struct componentname *cnp, struct vattr *vap, char *target)
{
	struct vop_symlink_args a;
	a.a_dvp = dvp;
	a.a_vpp = vpp;
	a.a_cnp = cnp;
	a.a_vap = vap;
	a.a_target = target;

	ASSERT_VP_ISLOCKED(dvp);

	if (dvp->v_op->vop_symlink == NULL)
		return ((dvp->v_op->vop_default)(&a));
	return ((dvp->v_op->vop_symlink)(&a));
}

int
VOP_READDIR(struct vnode *vp, struct uio *uio, struct ucred *cred, 
    int *eofflag, int *ncookies, u_long **cookies)
{
	struct vop_readdir_args a;
	a.a_vp = vp;
	a.a_uio = uio;
	a.a_cred = cred;
	a.a_eofflag = eofflag;
	a.a_ncookies = ncookies;
	a.a_cookies = cookies;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_readdir == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_readdir)(&a));
}

int
VOP_READLINK(struct vnode *vp, struct uio *uio, struct ucred *cred)
{
	struct vop_readlink_args a;
	a.a_vp = vp;
	a.a_uio = uio;
	a.a_cred = cred;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_readlink == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_readlink)(&a));
}

int
VOP_ABORTOP(struct vnode *dvp, struct componentname *cnp)
{
	struct vop_abortop_args a;
	a.a_dvp = dvp;
	a.a_cnp = cnp;

	if (dvp->v_op->vop_abortop == NULL)
		return ((dvp->v_op->vop_default)(&a));
	return ((dvp->v_op->vop_abortop)(&a));
}

int
VOP_INACTIVE(struct vnode *vp, struct proc *p)
{
	struct vop_inactive_args a;
	a.a_vp = vp;
	a.a_p = p;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_inactive == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_inactive)(&a));
}

int
VOP_RECLAIM(struct vnode *vp, struct proc *p)
{
	struct vop_reclaim_args a;
	a.a_vp = vp;
	a.a_p = p;

	if (vp->v_op->vop_reclaim == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_reclaim)(&a));
}

int
VOP_LOCK(struct vnode *vp, int flags, struct proc *p)
{
	struct vop_lock_args a;
	a.a_vp = vp;
	a.a_flags = flags;
	a.a_p = p;

	if (vp->v_op->vop_lock == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_lock)(&a));
}

int
VOP_UNLOCK(struct vnode *vp, int flags, struct proc *p)
{
	struct vop_unlock_args a;
	a.a_vp = vp;
	a.a_flags = flags;
	a.a_p = p;

	if (vp->v_op->vop_unlock == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_unlock)(&a));
}

int
VOP_BMAP(struct vnode *vp, daddr64_t bn, struct vnode **vpp, 
    daddr64_t *bnp, int *runp)
{
	struct vop_bmap_args a;
	a.a_vp = vp;
	a.a_bn = bn;
	a.a_vpp = vpp;
	a.a_bnp = bnp;
	a.a_runp = runp;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_bmap == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_bmap)(&a));
}

int
VOP_PRINT(struct vnode *vp)
{
	struct vop_print_args a;
	a.a_vp = vp;

	if (vp->v_op->vop_print == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_print)(&a));
}

int
VOP_PATHCONF(struct vnode *vp, int name, register_t *retval)
{
	struct vop_pathconf_args a;
	a.a_vp = vp;
	a.a_name = name;
	a.a_retval = retval;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_pathconf == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_pathconf)(&a));
}

int
VOP_ADVLOCK(struct vnode *vp, void *id, int op, struct flock *fl, int flags)
{
	struct vop_advlock_args a;
	a.a_vp = vp;
	a.a_id = id;
	a.a_op = op;
	a.a_fl = fl;
	a.a_flags = flags;

	if (vp->v_op->vop_advlock == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_advlock)(&a));
}

int
VOP_REALLOCBLKS(struct vnode *vp, struct cluster_save *buflist)
{
	struct vop_reallocblks_args a;
	a.a_vp = vp;
	a.a_buflist = buflist;

	ASSERT_VP_ISLOCKED(vp);

	if (vp->v_op->vop_reallocblks == NULL)
		return ((vp->v_op->vop_default)(&a));
	return ((vp->v_op->vop_reallocblks)(&a));
}

int
VOP_STRATEGY(struct buf *bp)
{
	struct vop_strategy_args a;
	a.a_bp = bp;

	if (bp->b_vp->v_op->vop_strategy == NULL)
		return ((bp->b_vp->v_op->vop_default)(&a));
	return ((bp->b_vp->v_op->vop_strategy)(&a));
}

int
VOP_BWRITE(struct buf *bp)
{
	struct vop_bwrite_args a;
	a.a_bp = bp;

	if (bp->b_vp->v_op->vop_bwrite == NULL)
		return ((bp->b_vp->v_op->vop_default)(&a));
	return ((bp->b_vp->v_op->vop_bwrite)(&a));
}
/* End of special cases. */
