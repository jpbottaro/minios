#include "gdt.h"

gdt_entry gdt[GDT_COUNT] = {
	(gdt_entry){(unsigned int) 0x00000000, (unsigned int) 0x00000000},
	(gdt_entry){ // code
		(unsigned short) 0xffff,
		(unsigned short) 0x0000,
		(unsigned char)  0x00,
        (unsigned char)  0x0a,      // Excecute/Read (code)
        (unsigned char)  0x01,      // code or data
        (unsigned char)  0x00,      // priv 0
        (unsigned char)  0x01, 
        (unsigned char)  0x0f, 
        (unsigned char)  0x00, 
        (unsigned char)  0x00, 
        (unsigned char)  0x01,
        (unsigned char)  0x01,
        (unsigned char)  0x00
	},
	(gdt_entry){ // data
		(unsigned short) 0xffff,
		(unsigned short) 0,
		(unsigned char)  0,
        (unsigned char)  2,         // Read/Write (data)
        (unsigned char)  1,         // code or data
        (unsigned char)  0,         // priv 0
        (unsigned char)  1, 
        (unsigned char)  0x0f, 
        (unsigned char)  0, 
        (unsigned char)  0, 
        (unsigned char)  1,
        (unsigned char)  1,
        (unsigned char)  0
	},
	(gdt_entry){ // video
		(unsigned short) 0x0f9f,    // 80x25chars * 2 bytes - 1
		(unsigned short) 0x8000,
		(unsigned char)  0xb,
        (unsigned char)  2,         // Read/Write (data)
        (unsigned char)  1,         // code or data
        (unsigned char)  0,         // priv 0
        (unsigned char)  1, 
        (unsigned char)  0,
        (unsigned char)  0, 
        (unsigned char)  0, 
        (unsigned char)  1,
        (unsigned char)  0,         // NOT 4 chunks
        (unsigned char)  0
	}
};

gdt_descriptor GDT_DESC = {sizeof(gdt)-1, (unsigned int) &gdt};

unsigned int gdt_free = (unsigned int) &gdt + 0x18;  // donde esta mapeada la memoria de video

unsigned int gdt_free_entry()
{
    gdt_free += 8;
    return gdt_free;
}

unsigned int gdt_desc(unsigned int ptr)
{
    return ptr - (unsigned int) &gdt;
}
