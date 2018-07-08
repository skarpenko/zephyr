zephyr_compile_options(-mips1 -mel -march=r3000 -msoft-float)
zephyr_ld_options(-mips1 -mel -march=r3000 -msoft-float)

zephyr_sources(
	prep_c.c
	reset.S
	cpu_idle.c
	swap.S
	thread.c
	irq_spurious.c
	exception.S
	fatal.c
)

zephyr_sources_if_kconfig(irq_offload.c)
