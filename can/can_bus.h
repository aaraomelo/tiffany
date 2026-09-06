/*
 * can_bus.h — modelo do barramento CAN multi-no
 *
 * Conhece apenas CanController / CanFrame.
 * NAO conhece J1939 nem micro.
 *
 * Regra central: arbitragem bit a bit.
 *   dominante = 0  vence  recessivo = 1
 *   menor ID numerico => maior prioridade
 *
 * Uso tipico:
 *   CanBus bus;
 *   can_bus_init(&bus);
 *   can_bus_attach(&bus, &ecu1);
 *   can_bus_attach(&bus, &ecu2);
 *   can_tx(&ecu1, &f1);
 *   can_tx(&ecu2, &f2);
 *   can_bus_tick(&bus);   // arbitra, entrega ao vencedor e aos ouvintes
 */

#ifndef CAN_BUS_H
#define CAN_BUS_H

#include "can.h"

#define CAN_BUS_MAX_NODES 16

typedef struct {
    CanController *node[CAN_BUS_MAX_NODES];
    int            n_nodes;
    CanFrame       last_won;     /* ultimo frame que venceu arbitragem */
    int            last_winner;  /* indice do no vencedor, -1 se ninguem */
} CanBus;

void can_bus_init(CanBus *bus);
int  can_bus_attach(CanBus *bus, CanController *node);

/*
 * Um "tick" do barramento:
 *  1. coleta nos com tx_pending e err_state != BUS_OFF
 *  2. arbitra bit a bit entre eles
 *  3. vencedor: can_tx_done + can_success_tx
 *  4. perdedores: lost_arbitration = 1 (mantem tx_pending para retentar)
 *  5. todos os nos (exceto se overflow) recebem o frame via inject_rx
 *
 * Retorna indice do vencedor, ou -1 se ninguem transmitia.
 */
int can_bus_tick(CanBus *bus);

/* quantos nos tem TX pendente */
int can_bus_tx_pending_count(const CanBus *bus);

#endif /* CAN_BUS_H */
