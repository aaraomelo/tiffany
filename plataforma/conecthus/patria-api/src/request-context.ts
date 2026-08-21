import { AsyncLocalStorage } from 'async_hooks';

export class RequestContext {
  private static storage = new AsyncLocalStorage<string>();

  static run<T>(alias: string, fn: () => T): T {
    return this.storage.run(alias, fn);
  }

  static get alias(): string {
    return this.storage.getStore() ?? 'patria';
  }
}
