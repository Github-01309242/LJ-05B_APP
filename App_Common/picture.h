#ifndef _PICTURE_H_
#define _PICTURE_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	const uint8_t *picture;
	uint16_t width;
	uint16_t height;
} PictureDesc_t;

#endif
