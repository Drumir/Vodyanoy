#ifndef FIFO__H
#define FIFO__H

//������ ������ ���� �������� ������: 4,8,16,32...128
#define FIFO( size )\
struct {\
	unsigned char buf[size];\
	unsigned char tail;\
	unsigned char head;\
}

//���������� ��������� � �������
#define FIFO_COUNT(fifo)     (fifo.head-fifo.tail)

//������ fifo
#define FIFO_SIZE(fifo)      ( sizeof(fifo.buf)/sizeof(fifo.buf[0]) )

//fifo ���������?
#define FIFO_IS_FULL(fifo)   (FIFO_COUNT(fifo)==FIFO_SIZE(fifo))

//fifo �����?
#define FIFO_IS_EMPTY(fifo)  (fifo.tail==fifo.head)

//���������� ���������� ����� � fifo
#define FIFO_SPACE(fifo)     (FIFO_SIZE(fifo)-FIFO_COUNT(fifo))

//��������� ������� � fifo
#define FIFO_PUSH(fifo, byte) \
{\
	fifo.buf[fifo.head & (FIFO_SIZE(fifo)-1)]=byte;\
	fifo.head++;\
}

//���� ������ ������� �� fifo
#define FIFO_FRONT(fifo) (fifo.buf[fifo.tail & (FIFO_SIZE(fifo)-1)])

//��������� ���������� ��������� � �������
#define FIFO_POP(fifo)   \
{\
	fifo.tail++; \
}

//�������� fifo
#define FIFO_FLUSH(fifo)   \
{\
	fifo.tail=0;\
	fifo.head=0;\
}


void uart_init(void);
int uart_send(char *str);
int uart_send_wo_CRLF(char *str);
int uart_sendPM(const char *str);
int uart_sendPM_wo_CRLF(const char *str);


#endif //FIFO__H