import { Injectable, Logger } from '@nestjs/common';
import * as nodemailer from 'nodemailer';

@Injectable()
export class MailService {
  private readonly logger = new Logger(MailService.name);
  private transporter: nodemailer.Transporter | null = null;

  private get from() {
    return process.env.SMTP_FROM || process.env.SMTP_USER || 'no-reply@patriatechnology.com';
  }

  private getTransport(): nodemailer.Transporter | null {
    if (this.transporter) return this.transporter;
    const host = process.env.SMTP_HOST;
    const user = process.env.SMTP_USER;
    const pass = process.env.SMTP_PASSWORD;
    if (!host || !user || !pass) {
      this.logger.warn('SMTP não configurado — emails não serão enviados');
      return null;
    }
    const port = Number(process.env.SMTP_PORT || 587);
    this.transporter = nodemailer.createTransport({
      host,
      port,
      secure: port === 465,
      auth: { user, pass },
    });
    return this.transporter;
  }

  async send(to: string, subject: string, html: string) {
    const tx = this.getTransport();
    if (!tx) {
      this.logger.warn(`(SMTP off) email para ${to} não enviado: ${subject}`);
      return;
    }
    await tx.sendMail({ from: this.from, to, subject, html });
  }

  async sendPasswordReset(to: string, name: string, link: string) {
    const html = `
      <div style="font-family: system-ui, Arial, sans-serif; max-width: 480px; margin: 0 auto; color: #222;">
        <h2 style="color:#1F4E79;">Redefinição de senha</h2>
        <p>Olá ${name || ''},</p>
        <p>Recebemos um pedido para redefinir a sua senha. Clique no botão abaixo para criar uma nova senha:</p>
        <p style="text-align:center; margin: 28px 0;">
          <a href="${link}" style="background:#1F4E79; color:#fff; padding:12px 24px; border-radius:6px; text-decoration:none; font-weight:600;">Redefinir senha</a>
        </p>
        <p style="font-size:13px; color:#666;">Se você não pediu isso, pode ignorar este email. O link expira em 1 hora.</p>
        <hr style="border:none; border-top:1px solid #eee; margin:24px 0;">
        <p style="font-size:12px; color:#999;">Patria Technology</p>
      </div>`;
    await this.send(to, 'Redefinição de senha', html);
  }
}
