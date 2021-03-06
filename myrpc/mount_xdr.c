/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "mount.h"

bool_t
xdr_fhandle(xdrs, objp)
	register XDR *xdrs;
	fhandle objp;
{

	register long *buf;

	if (!xdr_opaque(xdrs, objp, FHSIZE))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_fhandle3(xdrs, objp)
	register XDR *xdrs;
	fhandle3 *objp;
{

	register long *buf;

	if (!xdr_bytes(xdrs, (char **)&objp->fhandle3_val, (u_int *) &objp->fhandle3_len, FHSIZE3))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_fhstatus(xdrs, objp)
	register XDR *xdrs;
	fhstatus *objp;
{

	register long *buf;

	if (!xdr_u_int(xdrs, &objp->fhs_status))
		return (FALSE);
	switch (objp->fhs_status) {
	case 0:
		if (!xdr_fhandle(xdrs, objp->fhstatus_u.fhs_fhandle))
			return (FALSE);
		break;
	}
	return (TRUE);
}

#define	fhs_fh	fhstatus_u.fhs_fhandle

bool_t
xdr_mountstat3(xdrs, objp)
	register XDR *xdrs;
	mountstat3 *objp;
{

	register long *buf;

	if (!xdr_enum(xdrs, (enum_t *)objp))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_mountres3_ok(xdrs, objp)
	register XDR *xdrs;
	mountres3_ok *objp;
{

	register long *buf;

	if (!xdr_fhandle3(xdrs, &objp->fhandle))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->auth_flavors.auth_flavors_val, (u_int *) &objp->auth_flavors.auth_flavors_len, ~0,
		sizeof (int), (xdrproc_t) xdr_int))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_mountres3(xdrs, objp)
	register XDR *xdrs;
	mountres3 *objp;
{

	register long *buf;

	if (!xdr_mountstat3(xdrs, &objp->fhs_status))
		return (FALSE);
	switch (objp->fhs_status) {
	case MNT_OK:
		if (!xdr_mountres3_ok(xdrs, &objp->mountres3_u.mountinfo))
			return (FALSE);
		break;
	}
	return (TRUE);
}

bool_t
xdr_dirpath(xdrs, objp)
	register XDR *xdrs;
	dirpath *objp;
{

	register long *buf;

	if (!xdr_string(xdrs, objp, MNTPATHLEN))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_name(xdrs, objp)
	register XDR *xdrs;
	name *objp;
{

	register long *buf;

	if (!xdr_string(xdrs, objp, MNTNAMLEN))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_mountlist(xdrs, objp)
	register XDR *xdrs;
	mountlist *objp;
{

	register long *buf;

	if (!xdr_pointer(xdrs, (char **)objp, sizeof (struct mountbody), (xdrproc_t) xdr_mountbody))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_mountbody(xdrs, objp)
	register XDR *xdrs;
	mountbody *objp;
{

	register long *buf;

	if (!xdr_name(xdrs, &objp->ml_hostname))
		return (FALSE);
	if (!xdr_dirpath(xdrs, &objp->ml_directory))
		return (FALSE);
	if (!xdr_mountlist(xdrs, &objp->ml_next))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_groups(xdrs, objp)
	register XDR *xdrs;
	groups *objp;
{

	register long *buf;

	if (!xdr_pointer(xdrs, (char **)objp, sizeof (struct groupnode), (xdrproc_t) xdr_groupnode))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_groupnode(xdrs, objp)
	register XDR *xdrs;
	groupnode *objp;
{

	register long *buf;

	if (!xdr_name(xdrs, &objp->gr_name))
		return (FALSE);
	if (!xdr_groups(xdrs, &objp->gr_next))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_exports(xdrs, objp)
	register XDR *xdrs;
	exports *objp;
{

	register long *buf;

	if (!xdr_pointer(xdrs, (char **)objp, sizeof (struct exportnode), (xdrproc_t) xdr_exportnode))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_exportnode(xdrs, objp)
	register XDR *xdrs;
	exportnode *objp;
{

	register long *buf;

	if (!xdr_dirpath(xdrs, &objp->ex_dir))
		return (FALSE);
	if (!xdr_groups(xdrs, &objp->ex_groups))
		return (FALSE);
	if (!xdr_exports(xdrs, &objp->ex_next))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_ppathcnf(xdrs, objp)
	register XDR *xdrs;
	ppathcnf *objp;
{

	register long *buf;

	int i;

	if (xdrs->x_op == XDR_ENCODE) {
		buf = XDR_INLINE(xdrs, 6 * BYTES_PER_XDR_UNIT);
		if (buf == NULL) {
			if (!xdr_int(xdrs, &objp->pc_link_max))
				return (FALSE);
			if (!xdr_short(xdrs, &objp->pc_max_canon))
				return (FALSE);
			if (!xdr_short(xdrs, &objp->pc_max_input))
				return (FALSE);
			if (!xdr_short(xdrs, &objp->pc_name_max))
				return (FALSE);
			if (!xdr_short(xdrs, &objp->pc_path_max))
				return (FALSE);
			if (!xdr_short(xdrs, &objp->pc_pipe_buf))
				return (FALSE);

		} else {
			IXDR_PUT_LONG(buf, objp->pc_link_max);
			IXDR_PUT_SHORT(buf, objp->pc_max_canon);
			IXDR_PUT_SHORT(buf, objp->pc_max_input);
			IXDR_PUT_SHORT(buf, objp->pc_name_max);
			IXDR_PUT_SHORT(buf, objp->pc_path_max);
			IXDR_PUT_SHORT(buf, objp->pc_pipe_buf);
		}
		if (!xdr_u_char(xdrs, &objp->pc_vdisable))
			return (FALSE);
		if (!xdr_char(xdrs, &objp->pc_xxx))
			return (FALSE);
		buf = XDR_INLINE(xdrs, (2) * BYTES_PER_XDR_UNIT);
		if (buf == NULL) {
			if (!xdr_vector(xdrs, (char *)objp->pc_mask, 2,
				sizeof (short), (xdrproc_t) xdr_short))
				return (FALSE);
		} else {
			{
				register short *genp;

				for (i = 0, genp = objp->pc_mask;
					i < 2; i++) {
					IXDR_PUT_SHORT(buf, *genp++);
				}
			}
		}
		return (TRUE);
	} else if (xdrs->x_op == XDR_DECODE) {
		buf = XDR_INLINE(xdrs, 6 * BYTES_PER_XDR_UNIT);
		if (buf == NULL) {
			if (!xdr_int(xdrs, &objp->pc_link_max))
				return (FALSE);
			if (!xdr_short(xdrs, &objp->pc_max_canon))
				return (FALSE);
			if (!xdr_short(xdrs, &objp->pc_max_input))
				return (FALSE);
			if (!xdr_short(xdrs, &objp->pc_name_max))
				return (FALSE);
			if (!xdr_short(xdrs, &objp->pc_path_max))
				return (FALSE);
			if (!xdr_short(xdrs, &objp->pc_pipe_buf))
				return (FALSE);

		} else {
			objp->pc_link_max = IXDR_GET_LONG(buf);
			objp->pc_max_canon = IXDR_GET_SHORT(buf);
			objp->pc_max_input = IXDR_GET_SHORT(buf);
			objp->pc_name_max = IXDR_GET_SHORT(buf);
			objp->pc_path_max = IXDR_GET_SHORT(buf);
			objp->pc_pipe_buf = IXDR_GET_SHORT(buf);
		}
		if (!xdr_u_char(xdrs, &objp->pc_vdisable))
			return (FALSE);
		if (!xdr_char(xdrs, &objp->pc_xxx))
			return (FALSE);
		buf = XDR_INLINE(xdrs, (2) * BYTES_PER_XDR_UNIT);
		if (buf == NULL) {
			if (!xdr_vector(xdrs, (char *)objp->pc_mask, 2,
				sizeof (short), (xdrproc_t) xdr_short))
				return (FALSE);
		} else {
			{
				register short *genp;

				for (i = 0, genp = objp->pc_mask;
					i < 2; i++) {
					*genp++ = IXDR_GET_SHORT(buf);
				}
			}
		}
		return (TRUE);
	}

	if (!xdr_int(xdrs, &objp->pc_link_max))
		return (FALSE);
	if (!xdr_short(xdrs, &objp->pc_max_canon))
		return (FALSE);
	if (!xdr_short(xdrs, &objp->pc_max_input))
		return (FALSE);
	if (!xdr_short(xdrs, &objp->pc_name_max))
		return (FALSE);
	if (!xdr_short(xdrs, &objp->pc_path_max))
		return (FALSE);
	if (!xdr_short(xdrs, &objp->pc_pipe_buf))
		return (FALSE);
	if (!xdr_u_char(xdrs, &objp->pc_vdisable))
		return (FALSE);
	if (!xdr_char(xdrs, &objp->pc_xxx))
		return (FALSE);
	if (!xdr_vector(xdrs, (char *)objp->pc_mask, 2,
		sizeof (short), (xdrproc_t) xdr_short))
		return (FALSE);
	return (TRUE);
}
