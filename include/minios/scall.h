#ifndef _SCALL_H
#define _SCALL_H

#define SCALL_REGISTER(nr, sys) (scall_register((nr), (void (*)()) (sys)))

void scall_register(int nr, void (*sys)());

#endif /* _SCALL_H */
