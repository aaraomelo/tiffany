import { Injectable, Logger } from '@nestjs/common';
import * as nodemailer from 'nodemailer';

@Injectable()
export class EmailService {
  private readonly logger = new Logger(EmailService.name);
  private transporter: nodemailer.Transporter | null = null;
  private readonly from = process.env.SMTP_FROM || 'Patria Technology <contato@patriatechnology.com>';

  private getTransporter(): nodemailer.Transporter | null {
    if (this.transporter) return this.transporter;
    const host = process.env.SMTP_HOST;
    const user = process.env.SMTP_USER;
    const pass = process.env.SMTP_PASSWORD;
    const port = parseInt(process.env.SMTP_PORT || '587', 10);
    if (!host || !user || !pass) {
      this.logger.warn('SMTP não configurado — emails não serão enviados');
      return null;
    }
    this.transporter = nodemailer.createTransport({
      host,
      port,
      secure: port === 465,
      auth: { user, pass },
    });
    return this.transporter;
  }

  async send(to: string, subject: string, html: string, text?: string): Promise<boolean> {
    const t = this.getTransporter();
    if (!t) return false;
    try {
      const info = await t.sendMail({
        from: this.from,
        to,
        subject,
        text: text || html.replace(/<[^>]+>/g, ''),
        html,
      });
      this.logger.log(`Email enviado: to=${to} subject="${subject}" id=${info.messageId}`);
      return true;
    } catch (e: any) {
      this.logger.error(`Falha ao enviar email para ${to}: ${e.message}`);
      return false;
    }
  }

  async sendVerification(to: string, name: string | null, verifyUrl: string): Promise<boolean> {
    const safeName = name || 'olá';
    const subject = 'Confirme seu email · Patria Technology';
    const html = `
<!doctype html>
<html><body style="font-family:-apple-system,Segoe UI,Roboto,sans-serif;max-width:560px;margin:0 auto;padding:32px 24px;color:#1a1a1a;background:#f6f7fa;">
  <div style="background:white;border-radius:12px;padding:32px;border:1px solid #e6e8f0;">
    <div style="display:flex;align-items:center;gap:10px;margin-bottom:24px;">
      <span style="display:inline-flex;align-items:center;justify-content:center;width:32px;height:32px;background:linear-gradient(135deg,#cc1144,#ff5577);color:white;font-weight:700;border-radius:6px;font-size:14px;">P</span>
      <strong style="font-size:18px;">Patria Technology</strong>
    </div>
    <h2 style="font-size:20px;margin:0 0 12px;color:#1a1a1a;">${safeName}, confirma seu email aqui</h2>
    <p style="color:#5b6378;line-height:1.55;font-size:15px;">
      Você criou uma conta na Patria Technology. Pra liberar todas as funcionalidades (upgrade pra Pro, recuperação de senha, alertas de cota), confirme seu email clicando no botão abaixo:
    </p>
    <p style="margin:28px 0;text-align:center;">
      <a href="${verifyUrl}" style="display:inline-block;background:#cc1144;color:white;padding:12px 28px;border-radius:6px;text-decoration:none;font-weight:600;font-size:15px;">Confirmar email</a>
    </p>
    <p style="color:#8a91a8;font-size:13px;line-height:1.5;">
      Ou copie e cole esta URL no navegador:<br>
      <code style="background:#f4f5fa;padding:4px 8px;border-radius:3px;font-size:12px;word-break:break-all;">${verifyUrl}</code>
    </p>
    <p style="color:#8a91a8;font-size:12px;margin-top:24px;border-top:1px solid #e6e8f0;padding-top:16px;">
      Esse link expira em 7 dias. Se você não criou essa conta, pode ignorar este email.
    </p>
  </div>
  <p style="text-align:center;color:#8a91a8;font-size:11px;margin-top:16px;">
    © Patria Technology · Otimização combinatorial via álgebras hipercomplexas generalizadas
  </p>
</body></html>`;
    const text = `${safeName}, confirma seu email da conta Patria Technology.

Clique no link abaixo (expira em 7 dias):
${verifyUrl}

Se você não criou essa conta, ignore este email.`;
    return this.send(to, subject, html, text);
  }
}
