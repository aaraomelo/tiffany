// Templates HTML pra emails transacionais de billing.
// Extraídos do controller pra deixar o código de fluxo limpo.

import { SUPPORT_EMAIL } from './billing.config';

const SHELL_OPEN = `<!doctype html>
<html><body style="font-family:-apple-system,Segoe UI,Roboto,sans-serif;max-width:560px;margin:0 auto;padding:32px 24px;color:#1a1a1a;background:#f6f7fa;">
  <div style="background:white;border-radius:12px;padding:32px;border:1px solid #e6e8f0;">
    <div style="display:flex;align-items:center;gap:10px;margin-bottom:24px;">
      <span style="display:inline-flex;align-items:center;justify-content:center;width:32px;height:32px;background:linear-gradient(135deg,#cc1144,#ff5577);color:white;font-weight:700;border-radius:6px;font-size:14px;">P</span>
      <strong style="font-size:18px;">Patria Technology</strong>
    </div>`;
const SHELL_CLOSE = `</div></body></html>`;

// Email pro user confirmando upgrade Starter→Pro grátis (MVP)
export function upgradeToProEmail(opts: {
  userName?: string | null;
  tenantCompanyName: string;
  renewsAt: Date;
}): string {
  return `${SHELL_OPEN}
    <h2 style="font-size:20px;margin:0 0 12px;">Você está no plano Pro 🚀</h2>
    <p style="color:#5b6378;line-height:1.55;font-size:15px;">
      Olá ${opts.userName || ''}, seu tenant <strong>${opts.tenantCompanyName}</strong> agora está no <strong>Pro</strong>.
    </p>
    <ul style="color:#5b6378;font-size:14px;line-height:1.7;">
      <li>2.000 nós/s sustentado · burst 10.000</li>
      <li>n ≤ 2.000 vértices por chamada</li>
      <li>rollouts ≤ 64</li>
    </ul>
    <p style="color:#5b6378;font-size:14px;">
      Renovação automática em <strong>${opts.renewsAt.toLocaleDateString('pt-BR')}</strong>. Plano grátis durante o MVP — quando ativarmos cobrança, avisamos com antecedência.
    </p>
  ${SHELL_CLOSE}`;
}

// Notificação interna (contato@) quando chega um pedido de upgrade
export function upgradeRequestInternalEmail(opts: {
  planRequested: string;
  tenantCompanyName: string;
  tenantAlias: string;
  contactName?: string | null;
  ownerName?: string | null;
  ownerEmail?: string | null;
  phone?: string | null;
  whatsappConsent: boolean;
  cnpj?: string | null;
  companyName?: string | null;
  useCase?: string | null;
  estimatedVolume?: string | null;
  contactId: number | string;
}): string {
  const safeUseCase = opts.useCase ? opts.useCase.replace(/\n/g, '<br>') : '';
  return `<!doctype html><html><body style="font-family:sans-serif;max-width:560px;padding:24px;color:#1a1a1a;">
  <h2 style="margin:0 0 12px;">Novo pedido de upgrade · ${opts.planRequested.toUpperCase()}</h2>
  <p><strong>Tenant:</strong> ${opts.tenantCompanyName} (<code>${opts.tenantAlias}</code>)</p>
  <p><strong>Contato responsável:</strong> ${opts.contactName || opts.ownerName || '—'}<br>
     <strong>Email:</strong> ${opts.ownerEmail || '—'}<br>
     <strong>Telefone:</strong> ${opts.phone || '—'}${opts.whatsappConsent ? ' (WhatsApp OK)' : ''}</p>
  <p><strong>CNPJ:</strong> <code>${opts.cnpj || '—'}</code><br>
     <strong>Razão social:</strong> ${opts.companyName || opts.tenantCompanyName}</p>
  ${safeUseCase ? `<p><strong>Caso de uso:</strong><br>${safeUseCase}</p>` : ''}
  ${opts.estimatedVolume ? `<p><strong>Volume estimado:</strong> ${opts.estimatedVolume}</p>` : ''}
  <hr><p style="color:#888;font-size:12px;">Contact ID: ${opts.contactId} · ${new Date().toLocaleString('pt-BR')}</p>
</body></html>`;
}

// Confirmação automática pro cliente que pediu upgrade
export function upgradeRequestConfirmationEmail(opts: {
  contactName?: string | null;
  ownerName?: string | null;
  planRequested: string;
  companyName: string;
  contactId: number | string;
  cnpj?: string | null;
}): string {
  return `${SHELL_OPEN}
    <h2 style="font-size:20px;margin:0 0 12px;">Recebemos seu pedido de upgrade</h2>
    <p style="color:#5b6378;line-height:1.55;font-size:15px;">
      Olá ${opts.contactName || opts.ownerName || ''}, recebemos sua solicitação de upgrade pra
      <strong>${opts.planRequested.toUpperCase()}</strong> da <strong>${opts.companyName}</strong>.
    </p>
    <p style="color:#5b6378;line-height:1.55;font-size:15px;">
      Vamos analisar seus dados e entrar em contato em <strong>até 24 horas úteis</strong>
      por este mesmo email pra finalizar a contratação. Se precisar adiantar algo, responde direto pra
      <a href="mailto:${SUPPORT_EMAIL}" style="color:#cc1144;">${SUPPORT_EMAIL}</a>.
    </p>
    <div style="background:#f4f5fa;border-radius:6px;padding:14px 16px;margin:20px 0;font-family:ui-monospace,monospace;font-size:13px;color:#5b6378;">
      ID do pedido: ${opts.contactId}<br>
      Plano: ${opts.planRequested}<br>
      Empresa: ${opts.companyName}${opts.cnpj ? `<br>CNPJ: ${opts.cnpj}` : ''}
    </div>
    <p style="color:#8a91a8;font-size:12px;margin-top:24px;border-top:1px solid #e6e8f0;padding-top:16px;">
      Você não vai receber mais nenhum email automático sobre este pedido — apenas o retorno humano em até 24h.
    </p>
  ${SHELL_CLOSE}`;
}
