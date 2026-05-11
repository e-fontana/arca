#include "uart_protocol.h"
#include <string.h>

typedef enum {
    S_WAIT_START,
    S_WAIT_LEN,
    S_RECV_DATA,
    S_WAIT_PARITY,
    S_WAIT_STOP
} ParserState_t;

static UART_HandleTypeDef *_huart      = NULL;
static ParserState_t       _state      = S_WAIT_START;
static Protocol_Frame_t    _rx_frame;
static uint8_t             _rx_byte;
static uint8_t             _data_idx;
static uint8_t             _parity_calc;
static volatile uint8_t    _frame_ready = 0;

void Protocol_Init(UART_HandleTypeDef *huart)
{
    _huart       = huart;
    _state       = S_WAIT_START;
    _frame_ready = 0;
    HAL_UART_Receive_IT(_huart, &_rx_byte, 1);
}

/* Chamada pela ISR a cada byte recebido */
void Protocol_UART_RxCallback(void)
{
    switch (_state) {
        case S_WAIT_START:
            if (_rx_byte == PROTO_START) _state = S_WAIT_LEN;
            break;

        case S_WAIT_LEN:
            if (_rx_byte > 0 && _rx_byte <= PROTO_MAX_DATA) {
                _rx_frame.length = _rx_byte;
                _data_idx        = 0;
                _parity_calc     = 0;
                _state           = S_RECV_DATA;
            } else {
                _state = S_WAIT_START;
            }
            break;

        case S_RECV_DATA:
            _rx_frame.data[_data_idx++] = _rx_byte;
            _parity_calc ^= _rx_byte;
            if (_data_idx == _rx_frame.length) _state = S_WAIT_PARITY;
            break;

        case S_WAIT_PARITY:
            _rx_frame.valid = (_rx_byte == _parity_calc) ? 1 : 0;
            _state = S_WAIT_STOP;
            break;

        case S_WAIT_STOP:
            if (_rx_byte == PROTO_STOP && _rx_frame.valid) {
                _frame_ready = 1;
            } else {
                _rx_frame.valid = 0;
            }
            _state = S_WAIT_START;
            break;
    }

    /* Re-arma para o próximo byte */
    HAL_UART_Receive_IT(_huart, &_rx_byte, 1);
}

uint8_t Protocol_Receive(Protocol_Frame_t *dst)
{
    if (!_frame_ready) return 0;
    *dst = _rx_frame;
    _frame_ready = 0;
    return 1;
}

uint8_t Protocol_Send(const uint8_t *data, uint8_t length)
{
    if (length == 0 || length > PROTO_MAX_DATA) return 0;

    uint8_t parity = 0;
    for (uint8_t i = 0; i < length; i++) parity ^= data[i];

    uint8_t frame[PROTO_MAX_DATA + 4];
    frame[0] = PROTO_START;
    frame[1] = length;
    for (uint8_t i = 0; i < length; i++) frame[2 + i] = data[i];
    frame[2 + length] = parity;
    frame[3 + length] = PROTO_STOP;

    return (HAL_UART_Transmit(_huart, frame, (uint16_t)(length + 4), 200) == HAL_OK) ? 1 : 0;
}
