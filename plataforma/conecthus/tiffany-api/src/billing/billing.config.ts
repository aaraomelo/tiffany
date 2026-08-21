// Configuração PagBank — leitura única de env vars na startup.
// Centraliza pra evitar re-leitura espalhada pelo controller.

export const SANDBOX = process.env.PAGBANK_SANDBOX === 'true';

export const PAGBANK_TOKEN = SANDBOX
  ? process.env.PAGBANK_TOKEN_SANDBOX || ''
  : process.env.PAGBANK_TOKEN_LIVE || '';

export const PAGBANK_BASE_URL = SANDBOX
  ? process.env.PAGBANK_BASE_URL_SANDBOX || 'https://sandbox.api.pagseguro.com'
  : process.env.PAGBANK_BASE_URL_LIVE || 'https://api.pagseguro.com';

export const PRO_AMOUNT_CENTS = parseInt(
  process.env.PAGBANK_PRO_AMOUNT_CENTS || '29900',
  10,
);

export const PUBLIC_BASE = process.env.PUBLIC_BASE_URL || 'https://nco.patriatechnology.com';

export const SUPPORT_EMAIL = process.env.SMTP_USER || 'contato@patriatechnology.com';

// Validade fixa de uma sub Pro: 30 dias
export const SUBSCRIPTION_PERIOD_MS = 30 * 24 * 3600 * 1000;

// Cooldown entre dois pedidos de upgrade do mesmo usuário (anti-spam de email)
export const UPGRADE_REQUEST_COOLDOWN_MS = 3600 * 1000;

// Placeholder de CPF válido aceito pela sandbox PagBank quando o cliente não informa
export const SANDBOX_PLACEHOLDER_CPF = '12345678909';

// Item single-source-of-truth do plano Pro mensal
export const PRO_ITEM = {
  reference_id: 'patria-nco-pro-monthly',
  name: 'Patria NCO API · Plano Pro · 1 mês',
};

export const PIX_EXPIRATION_MS = 30 * 60 * 1000; // 30min
