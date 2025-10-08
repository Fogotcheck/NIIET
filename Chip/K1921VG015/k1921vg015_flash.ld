OUTPUT_ARCH( "riscv" )
ENTRY(_start)

_estack = ORIGIN(REGION_STACK) + LENGTH(REGION_STACK);
_Min_Heap_Size = LENGTH(REGION_HEAP); /* required amount of heap */

MEMORY {
  FLASH  (rx)  : ORIGIN = 0x80000000, LENGTH = 1024K
  RAM    (rwx) : ORIGIN = 0x40000000, LENGTH = 256K
  CCMRAM (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
}

REGION_ALIAS("REGION_TEXT",   FLASH );
REGION_ALIAS("REGION_RODATA", FLASH );
REGION_ALIAS("REGION_DATA",   RAM);
REGION_ALIAS("REGION_BSS",    RAM);
REGION_ALIAS("REGION_STACK",  RAM);
REGION_ALIAS("REGION_HEAP",   CCMRAM);

STACK_SIZE = 2048;

/*
*  @brief Common part of bare metal linker script
*/

/*
*  Some external definitions are requeued:
* - output format, arch and entry point (OUTPUT_FORMAT, OUTPUT_ARCH, ENTRY commands)
* - memory layout (MEMORY command)
* - memory regions' aliases (REGION_ALIAS comand)
* - stack size (STACK_SIZE symbol, i.e. "STACK_SIZE = 2048;")
* - size of heap ("HEAP_FIXED_AFTER_BSS=<size>", default it is a space between end of .bss and start of .stack)
*/

STACK_SIZE = DEFINED(STACK_SIZE) ? STACK_SIZE : 2048;

SECTIONS {
  /* startup/crt code segment */
  .text.startup ORIGIN(REGION_TEXT) : {
    *(.startup.entry .startup.*)
    . = ALIGN(16);
    *(.init.rodata .init.rodata.*)
    . = ALIGN(16);
    *(.init.data .init.data.*)
    . = ALIGN(16);
    *(.init.text .init.text.*)
  } >REGION_TEXT

  __TEXT_INIT_START__ = LOADADDR(.text.startup);
  __TEXT_INIT_SIZE__ = SIZEOF(.text.startup);

  .text.crt : ALIGN(16) {
    *(.text.crt*)
  } >REGION_TEXT

  /* code segment */
  .text : ALIGN(4) {
    PROVIDE(__TEXT_START__ = .);
    *(.text .text.*)
    PROVIDE(__TEXT_END__ = .);
  } >REGION_TEXT

  .init_array : ALIGN(4) {
     KEEP(*(SORT(.init_array*)))
  } >REGION_DATA AT>REGION_TEXT

  .fini_array : ALIGN(4) {
     KEEP(*(SORT(.fini_array*)))
  } >REGION_DATA AT>REGION_TEXT

  /* thread-local data segment */
  .tdata : ALIGN(4) {
    PROVIDE(_tls_data = .);
    PROVIDE(_tdata_start = .);
    *(.tdata .tdata.*)
    PROVIDE(_tdata_end = .);
  } >REGION_RODATA

  .tbss : ALIGN(4) {
    PROVIDE(_tbss_start = .);
    *(.tbss .tbss.*)
    . = ALIGN(4);
    PROVIDE(_tbss_end = .);
  } >REGION_RODATA

  /* read-only data segment */
  .rodata : ALIGN(4) {
    *(.rodata) *(.rodata.*) *(.gnu.linkonce.r.*)
  } >REGION_RODATA

  /* small read-only data segment */
  .srodata : {
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata*)
  } >REGION_RODATA

  /* small data segment */
  .sdata : ALIGN(4) {
    __SDATA_BEGIN__ = .;
    *(.sdata .sdata.* .sdata2.* .gnu.linkonce.s.*)
  } >REGION_DATA AT>REGION_TEXT

  /* data segment */
  .data : ALIGN(4) {
    __DATA_BEGIN__ = .;
    *(.data .data.* .gnu.linkonce.d.*)
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */
    _edata = .; PROVIDE (edata = .);
  } >REGION_DATA AT>REGION_TEXT

  PROVIDE( __data_source_start = LOADADDR(.data) );
  PROVIDE( __data_target_start = ADDR(.data) );
  PROVIDE( __data_target_end = ADDR(.data) + SIZEOF(.data) );

  PROVIDE( __sdata_source_start = LOADADDR(.sdata) );
  PROVIDE( __sdata_target_start = ADDR(.sdata) );
  PROVIDE( __sdata_target_end = ADDR(.sdata) + SIZEOF(.sdata) );

  PROVIDE( __init_array_source_start = LOADADDR(.init_array) );
  PROVIDE( __init_array_target_start = ADDR(.init_array) );
  PROVIDE( __init_array_target_end = ADDR(.init_array) + SIZEOF(.init_array) );

  PROVIDE( __fini_array_source_start = LOADADDR(.fini_array) );
  PROVIDE( __fini_array_target_start = ADDR(.fini_array) );
  PROVIDE( __fini_array_target_end = ADDR(.fini_array) + SIZEOF(.fini_array) );

  /* bss segment */
  .sbss : {
    PROVIDE(__bss_start = .);
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
  } >REGION_BSS

  .bss : ALIGN(4) {
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(4);
    __BSS_END__ = .;
    PROVIDE(__bss_end = .);

  } >REGION_BSS

  /* End of uninitalized data segement */

  __global_pointer$ = MIN(__DATA_BEGIN__ + 0x780,
                          MAX(__SDATA_BEGIN__ + 0x780, __BSS_END__ - 0x780));

  /* User_heap_stack section, used to check that there is enough "RAM" Ram  type memory left */
  ._user_heap :
  {
    . = ALIGN(16);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    PROVIDE ( __end = . );
    . = . + _Min_Heap_Size;
  } >REGION_HEAP

  /* discard relocation code */
  /* plf_init_relocate = plf_init_noreloc;*/

  /DISCARD/ : {
    *(.init.text.plf_init_relocate)
    *(.eh_frame .eh_frame.*)
  }
}
