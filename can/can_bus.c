/*
 * can_bus.c — barramento CAN multi-no
 *
 * tick:
 *  1. candidatos com tx_pending e nao BUS_OFF
 *  2. arbitragem bit a bit
 *  3. ACK: precisa de >= 1 outro no nao BUS_OFF (ouvinte)
 *     - sem ACK -> can_error_tx(vencedor), status ACK_ERR, TX permanece
 *     - com ACK -> can_tx_done + can_success_tx, broadcast RX
 */

#include "can_bus.h"
#include <string.h>

void can_bus_init(CanBus *bus)
{
    memset(bus, 0, sizeof(*bus));
    bus->last_winner = -1;
}

int can_bus_attach(CanBus *bus, CanController *node)
{
    if (!bus || !node) return -1;
    if (bus->n_nodes >= CAN_BUS_MAX_NODES) return -1;
    bus->node[bus->n_nodes++] = node;
    return bus->n_nodes - 1;
}

int can_bus_tx_pending_count(const CanBus *bus)
{
    if (!bus) return 0;
    int n = 0;
    for (int i = 0; i < bus->n_nodes; i++) {
        CanController *c = bus->node[i];
        if (c && c->tx_pending && c->err_state != CAN_ERR_BUS_OFF)
            n++;
    }
    return n;
}

static int count_listeners(const CanBus *bus, int tx_idx)
{
    int n = 0;
    for (int i = 0; i < bus->n_nodes; i++) {
        if (i == tx_idx) continue;
        CanController *c = bus->node[i];
        if (c && c->err_state != CAN_ERR_BUS_OFF)
            n++;
    }
    return n;
}

int can_bus_tick(CanBus *bus)
{
    if (!bus || bus->n_nodes == 0) return -1;

    int candidates[CAN_BUS_MAX_NODES];
    int nc = 0;
    for (int i = 0; i < bus->n_nodes; i++) {
        CanController *c = bus->node[i];
        if (c && c->tx_pending && c->err_state != CAN_ERR_BUS_OFF)
            candidates[nc++] = i;
    }
    if (nc == 0) {
        bus->last_winner = -1;
        return -1;
    }

    int winner = candidates[0];
    for (int i = 1; i < nc; i++) {
        int j = candidates[i];
        int cmp = can_arbitrate(&bus->node[winner]->tx, &bus->node[j]->tx);
        if (cmp > 0)
            winner = j;
        /* empate (mesmo ID): mantem o primeiro candidato */
    }

    CanFrame won = bus->node[winner]->tx;
    bus->last_won = won;
    bus->last_winner = winner;

    for (int i = 0; i < nc; i++) {
        int j = candidates[i];
        if (j == winner) continue;
        bus->node[j]->lost_arbitration = 1;
    }

    /* ACK: precisa de ouvinte */
    int listeners = count_listeners(bus, winner);
    if (listeners == 0) {
        bus->node[winner]->ack_seen = 0;
        bus->node[winner]->status = CAN_ST_ACK_ERR;
        can_error_tx(bus->node[winner]);
        /* TX permanece pendente para retentar apos recuperacao */
        return winner;
    }

    bus->node[winner]->ack_seen = 1;
    can_tx_done(bus->node[winner]);
    can_success_tx(bus->node[winner]);

    for (int i = 0; i < bus->n_nodes; i++) {
        CanController *c = bus->node[i];
        if (!c || c->err_state == CAN_ERR_BUS_OFF) continue;
        if (i == winner) continue;
        if (c->rx_pending)
            c->status = CAN_ST_OVERFLOW;
        else
            can_inject_rx(c, &won);
    }

    return winner;
}
