import { Injectable } from '@nestjs/common';
import { readFileSync, writeFileSync, existsSync, mkdirSync } from 'fs';
import { join } from 'path';

export interface Task {
  id: string;
  status: 'pending' | 'planning' | 'needs_info' | 'awaiting_approval' | 'approved' | 'executing' | 'completed' | 'failed' | 'rejected';
  command: string;
  description: string;
  createdBy: string;
  channel?: string;
  target?: string;
  plan?: string;
  question?: string;
  feedback?: string;
  result?: string;
  createdAt: string;
  updatedAt: string;
}

@Injectable()
export class TasksService {
  private filePath = join(__dirname, '..', 'data', 'tasks.json');

  private readTasks(): Task[] {
    if (!existsSync(this.filePath)) return [];
    return JSON.parse(readFileSync(this.filePath, 'utf-8'));
  }

  private writeTasks(tasks: Task[]) {
    const dir = join(__dirname, '..', 'data');
    if (!existsSync(dir)) {
      mkdirSync(dir, { recursive: true });
    }
    writeFileSync(this.filePath, JSON.stringify(tasks, null, 2));
  }

  create(command: string, description: string, createdBy: string, channel?: string, target?: string): Task {
    const tasks = this.readTasks();
    const task: Task = {
      id: Date.now().toString(36) + Math.random().toString(36).slice(2, 6),
      status: 'pending',
      command,
      description,
      createdBy,
      channel: channel || 'whatsapp',
      target: target || '+5511977808883',
      createdAt: new Date().toISOString(),
      updatedAt: new Date().toISOString(),
    };
    tasks.push(task);
    this.writeTasks(tasks);
    return task;
  }

  findAll(status?: string): Task[] {
    const tasks = this.readTasks();
    if (status) return tasks.filter(t => t.status === status);
    return tasks;
  }

  findOne(id: string): Task | undefined {
    return this.readTasks().find(t => t.id === id);
  }

  update(id: string, data: Partial<Pick<Task, 'status' | 'result' | 'plan' | 'question' | 'feedback' | 'channel' | 'target'>>): Task | undefined {
    const tasks = this.readTasks();
    const task = tasks.find(t => t.id === id);
    if (!task) return undefined;
    if (data.status) task.status = data.status as any;
    if (data.result !== undefined) task.result = data.result;
    if (data.plan !== undefined) task.plan = data.plan;
    if (data.question !== undefined) task.question = data.question;
    if (data.feedback !== undefined) task.feedback = data.feedback;
    if (data.channel !== undefined) task.channel = data.channel;
    if (data.target !== undefined) task.target = data.target;
    task.updatedAt = new Date().toISOString();
    this.writeTasks(tasks);
    return task;
  }
}
