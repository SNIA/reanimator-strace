static int
arch_set_error(struct tcb *tcp)
{
	cris_r10 = -tcp->u_error;
	return upoke(tcp->pid, 4 * PT_R10, cris_r10);
}
