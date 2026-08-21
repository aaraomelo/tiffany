import { Injectable } from '@nestjs/common';
import { EventEmitter } from 'events';

export interface TaskEvent {
  type: 'created' | 'status_changed' | 'deploy_completed';
  taskId: string;
  status: string;
  previousStatus?: string;
}

@Injectable()
export class TaskEventsService {
  private emitter = new EventEmitter();

  constructor() {
    this.emitter.setMaxListeners(20);
  }

  emit(event: TaskEvent) {
    this.emitter.emit('task', event);
  }

  subscribe(listener: (event: TaskEvent) => void) {
    this.emitter.on('task', listener);
    return () => this.emitter.off('task', listener);
  }
}
