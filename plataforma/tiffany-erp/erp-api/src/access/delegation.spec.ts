import {
  UNLIMITED,
  attenuatedDepth,
  canDelegate,
  depthFromStored,
  depthToStored,
  maxDelegableDepth,
} from './delegation';

describe('delegação com profundidade limitada (δ)', () => {
  it('storage ↔ código: null ↔ ∞', () => {
    expect(depthFromStored(null)).toBe(UNLIMITED);
    expect(depthFromStored(undefined)).toBe(UNLIMITED);
    expect(depthFromStored(0)).toBe(0);
    expect(depthFromStored(3)).toBe(3);
    expect(depthToStored(UNLIMITED)).toBeNull();
    expect(depthToStored(0)).toBe(0);
    expect(depthToStored(2)).toBe(2);
  });

  it('δ=0 não pode delegar (não compartilha)', () => {
    expect(canDelegate(0)).toBe(false);
    expect(maxDelegableDepth(0)).toBeNull();
    expect(attenuatedDepth(0, 0)).toBeNull();
  });

  it('δ=1 só delega com δ=0 nos destinatários', () => {
    expect(canDelegate(1)).toBe(true);
    expect(maxDelegableDepth(1)).toBe(0);
    expect(attenuatedDepth(1, 0)).toBe(0);
    expect(attenuatedDepth(1, 1)).toBeNull();
  });

  it('δ=2 desce e decrementa (0 ou 1 permitidos)', () => {
    expect(maxDelegableDepth(2)).toBe(1);
    expect(attenuatedDepth(2, 0)).toBe(0);
    expect(attenuatedDepth(2, 1)).toBe(1);
    expect(attenuatedDepth(2, 2)).toBeNull();
  });

  it('∞ é irrestrito (∞−1=∞)', () => {
    expect(canDelegate(UNLIMITED)).toBe(true);
    expect(maxDelegableDepth(UNLIMITED)).toBe(UNLIMITED);
    expect(attenuatedDepth(UNLIMITED, 5)).toBe(5);
    expect(attenuatedDepth(UNLIMITED, UNLIMITED)).toBe(UNLIMITED);
  });

  it('rejeita profundidade negativa', () => {
    expect(attenuatedDepth(3, -1)).toBeNull();
  });
});
