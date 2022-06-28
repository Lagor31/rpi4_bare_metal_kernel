.global _wait_for_event  
.section .text
.type _wait_for_event, @function
_wait_for_event:
1:  wfe
    b       1b
.end