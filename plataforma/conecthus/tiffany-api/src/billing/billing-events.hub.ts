import { Injectable, MessageEvent } from '@nestjs/common';
import { Observable, Subject } from 'rxjs';

// Hub de eventos por tenant (in-memory). Patria-api é instância única, então
// um Map<tenantId, Subject> resolve. Em multi-instância, trocar por Redis pub/sub.
@Injectable()
export class BillingEventsHub {
  private subjects = new Map<string, Subject<MessageEvent>>();

  stream(tenantId: string): Observable<MessageEvent> {
    let s = this.subjects.get(tenantId);
    if (!s) {
      s = new Subject<MessageEvent>();
      this.subjects.set(tenantId, s);
    }
    return s.asObservable();
  }

  emit(tenantId: string, payload: { type: string; [k: string]: any }) {
    this.subjects.get(tenantId)?.next({
      data: JSON.stringify(payload),
      type: payload.type,
    });
  }

  cleanup(tenantId: string) {
    this.subjects.get(tenantId)?.complete();
    this.subjects.delete(tenantId);
  }
}
