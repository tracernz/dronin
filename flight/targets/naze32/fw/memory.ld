__main_stack_size__     = 0x0400;
__process_stack_size__  = 0x0400;

MEMORY
{
    flash : org = 0x08000000, len = 128k - 100 - 12k
    /* fw_info should be filled when the tlfw is generated */
    fw_info : org = 0x08000000 + 128k - 100 - 12k, len = 100
    /* Putting anything in EEPROM will overwrite saved settings */
    eeprom : org = 0x08000000 + 128k - 12k, len = 12k
    ram : org = 0x20000000, len = 20k
}
