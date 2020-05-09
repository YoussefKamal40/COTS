#ifndef _DMA_REGISTERS_H_
#define _DMA_REGISTERS_H_

#define DMA_ISR_GIF1 ((u32)0x1)
#define DMA_ISR_TCIF1 ((u32)0x2)
#define DMA_ISR_HTIF1 ((u32)0x4)
#define DMA_ISR_TEIF1 ((u32)0x8)
#define DMA_ISR_GIF2 ((u32)0x10)
#define DMA_ISR_TCIF2 ((u32)0x20)
#define DMA_ISR_HTIF2 ((u32)0x40)
#define DMA_ISR_TEIF2 ((u32)0x80)
#define DMA_ISR_GIF3 ((u32)0x100)
#define DMA_ISR_TCIF3 ((u32)0x200)
#define DMA_ISR_HTIF3 ((u32)0x400)
#define DMA_ISR_TEIF3 ((u32)0x800)
#define DMA_ISR_GIF4 ((u32)0x1000)
#define DMA_ISR_TCIF4 ((u32)0x2000)
#define DMA_ISR_HTIF4 ((u32)0x4000)
#define DMA_ISR_TEIF4 ((u32)0x8000)
#define DMA_ISR_GIF5 ((u32)0x10000)
#define DMA_ISR_TCIF5 ((u32)0x20000)
#define DMA_ISR_HTIF5 ((u32)0x40000)
#define DMA_ISR_TEIF5 ((u32)0x80000)
#define DMA_ISR_GIF6 ((u32)0x100000)
#define DMA_ISR_TCIF6 ((u32)0x200000)
#define DMA_ISR_HTIF6 ((u32)0x400000)
#define DMA_ISR_TEIF6 ((u32)0x800000)
#define DMA_ISR_GIF7 ((u32)0x1000000)
#define DMA_ISR_TCIF7 ((u32)0x2000000)
#define DMA_ISR_HTIF7 ((u32)0x4000000)
#define DMA_ISR_TEIF7 ((u32)0x8000000)

#define DMA_IFCR_CGIF1 ((u32)0x1)
#define DMA_IFCR_CGIF2 ((u32)0x10)
#define DMA_IFCR_CGIF3 ((u32)0x100)
#define DMA_IFCR_CGIF4 ((u32)0x1000)
#define DMA_IFCR_CGIF5 ((u32)0x10000)
#define DMA_IFCR_CGIF6 ((u32)0x100000)
#define DMA_IFCR_CGIF7 ((u32)0x1000000)
#define DMA_IFCR_CTCIF1 ((u32)0x2)
#define DMA_IFCR_CTCIF2 ((u32)0x20)
#define DMA_IFCR_CTCIF3 ((u32)0x200)
#define DMA_IFCR_CTCIF4 ((u32)0x2000)
#define DMA_IFCR_CTCIF5 ((u32)0x20000)
#define DMA_IFCR_CTCIF6 ((u32)0x200000)
#define DMA_IFCR_CTCIF7 ((u32)0x2000000)
#define DMA_IFCR_CHTIF1 ((u32)0x4)
#define DMA_IFCR_CHTIF2 ((u32)0x40)
#define DMA_IFCR_CHTIF3 ((u32)0x400)
#define DMA_IFCR_CHTIF4 ((u32)0x4000)
#define DMA_IFCR_CHTIF5 ((u32)0x40000)
#define DMA_IFCR_CHTIF6 ((u32)0x400000)
#define DMA_IFCR_CHTIF7 ((u32)0x4000000)
#define DMA_IFCR_CTEIF1 ((u32)0x8)
#define DMA_IFCR_CTEIF2 ((u32)0x80)
#define DMA_IFCR_CTEIF3 ((u32)0x800)
#define DMA_IFCR_CTEIF4 ((u32)0x8000)
#define DMA_IFCR_CTEIF5 ((u32)0x80000)
#define DMA_IFCR_CTEIF6 ((u32)0x800000)
#define DMA_IFCR_CTEIF7 ((u32)0x8000000)

#define DMA_CCR_EN ((u32)0x1)
#define DMA_CCR_TCIE ((u32)0x2)
#define DMA_CCR_HTIE ((u32)0x4)
#define DMA_CCR_TEIE ((u32)0x8)
#define DMA_CCR_DIR ((u32)0x10)
#define DMA_CCR_CIRC ((u32)0x20)
#define DMA_CCR_PINC ((u32)0x40)
#define DMA_CCR_MINC ((u32)0x80)
#define DMA_CCR_PSIZE_SETMASK ((u32)0x300)
#define DMA_CCR_PSIZE_CLEARMASK ((u32)0xfffffcff)
#define DMA_CCR_PSIZE_OFFSET ((u8)0x8)
#define DMA_CCR_MSIZE_SETMASK ((u32)0xc00)
#define DMA_CCR_MSIZE_CLEARMASK ((u32)0xfffff3ff)
#define DMA_CCR_MSIZE_OFFSET ((u8)0xa)
#define DMA_CCR_PL_SETMASK ((u32)0x3000)
#define DMA_CCR_PL_CLEARMASK ((u32)0xffffcfff)
#define DMA_CCR_PL_OFFSET ((u8)0xc)
#define DMA_CCR_MEM2MEM ((u32)0x4000)

#define DMA_CNDTR_NDT_SETMASK ((u32)0xffff)
#define DMA_CNDTR_NDT_CLEARMASK ((u32)0xffff0000)
#define DMA_CNDTR_NDT_OFFSET ((u8)0x0)

#define DMA_CPAR_PA_SETMASK ((u32)0xffffffff)
#define DMA_CPAR_PA_CLEARMASK ((u32)0x0)
#define DMA_CPAR_PA_OFFSET ((u8)0x0)

#define DMA_CMAR_MA_SETMASK ((u32)0xffffffff)
#define DMA_CMAR_MA_CLEARMASK ((u32)0x0)
#define DMA_CMAR_MA_OFFSET ((u8)0x0)

typedef struct DMAtype
{
	u32 ISR;
	u32 IFCR;
	struct
	{
		u32 CCR;
		u32 CNDTR;
		u32 CPAR;
		u32 CMAR;
		u32 reserved;
	}channel[7];
}DMAtype;

#endif