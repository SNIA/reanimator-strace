static int
arch_set_error(struct tcb *tcp)
{
	microblaze_r3 = -tcp->u_error;
	return upoke(tcp->pid, 3 * 4, microblaze_r3);
}
