MEMORY
{
  IAPL           (rx)   : ORIGIN = 0x08000000, LENGTH = 2K - 0x20    /* Preloader code (IAPL) */
  TOMBSTONE_IAPL (rx)   : ORIGIN = 0x080007E0, LENGTH = 0x20         /* Tombstone Preloader (IAPL) @ end of preloader section */
  TOMBSTONE_UAPP (rx)   : ORIGIN = 0x08000800, LENGTH = 0x20         /* Tombstone User Application (UAPP) @ before app code start */
  APP            (rx)   : ORIGIN = 0x08000A00, LENGTH = 32K - 0xA00  /* User Application (UAPP) */
  RAM            (xrw)  : ORIGIN = 0x20000000, LENGTH = 8K
}
