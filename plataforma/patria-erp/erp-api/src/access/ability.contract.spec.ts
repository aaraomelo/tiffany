import { buildAbility } from './ability.factory';
import { SYSTEM_ROLES } from './system-roles';
import type { Action, Subject } from './access.types';

// Trava de regressão: afirma a matriz de permissões PRETENDIDA de cada perfil
// de sistema. Mexeu numa regra do seed e quebrou em cadeia → teste vermelho.

function abilityFor(roleName: string) {
  const role = SYSTEM_ROLES.find((r) => r.name === roleName);
  if (!role) throw new Error(`perfil ${roleName} não existe`);
  return buildAbility([{ rules: role.rules }]);
}

const can = (a: ReturnType<typeof abilityFor>, act: Action, sub: Subject) => a.can(act, sub);

describe('contrato dos perfis de sistema', () => {
  describe('Administrador', () => {
    const a = abilityFor('Administrador');
    it('faz tudo', () => {
      expect(can(a, 'delete', 'Order')).toBe(true);
      expect(can(a, 'manage', 'Role')).toBe(true);
      expect(can(a, 'create', 'User')).toBe(true);
      expect(can(a, 'update', 'Theme')).toBe(true);
    });
  });

  describe('Gerente', () => {
    const a = abilityFor('Gerente');
    it('gerencia operações', () => {
      expect(can(a, 'manage', 'Customer')).toBe(true);
      expect(can(a, 'delete', 'Order')).toBe(true);
      expect(can(a, 'create', 'Student')).toBe(true);
    });
    it('vê o controle de acesso mas não o altera', () => {
      expect(can(a, 'read', 'Role')).toBe(true);
      expect(can(a, 'read', 'User')).toBe(true);
      expect(can(a, 'create', 'Role')).toBe(false);
      expect(can(a, 'update', 'User')).toBe(false);
    });
  });

  describe('Operador', () => {
    const a = abilityFor('Operador');
    it('cria/lê/edita o operacional do dia a dia', () => {
      expect(can(a, 'create', 'Order')).toBe(true);
      expect(can(a, 'update', 'Customer')).toBe(true);
      expect(can(a, 'read', 'Tuition')).toBe(true);
    });
    it('não deleta o operacional', () => {
      expect(can(a, 'delete', 'Order')).toBe(false);
      expect(can(a, 'delete', 'Customer')).toBe(false);
    });
    it('só lê estoque/carteira/planos', () => {
      expect(can(a, 'read', 'Stock')).toBe(true);
      expect(can(a, 'update', 'Stock')).toBe(false);
      expect(can(a, 'read', 'Wallet')).toBe(true);
      expect(can(a, 'create', 'Wallet')).toBe(false);
    });
    it('não acessa administração', () => {
      expect(can(a, 'read', 'Role')).toBe(false);
      expect(can(a, 'read', 'User')).toBe(false);
    });
  });

  describe('Somente leitura', () => {
    const a = abilityFor('Somente leitura');
    it('lê tudo', () => {
      expect(can(a, 'read', 'Order')).toBe(true);
      expect(can(a, 'read', 'Student')).toBe(true);
    });
    it('não altera nada', () => {
      expect(can(a, 'create', 'Order')).toBe(false);
      expect(can(a, 'update', 'Student')).toBe(false);
      expect(can(a, 'delete', 'Customer')).toBe(false);
    });
  });
});
