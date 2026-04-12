import { Injectable, Logger } from '@nestjs/common';
import { PrismaService } from '../prisma.service';
import { EvolutionApiService } from './evolution-api.service';
import { TelegramService } from './telegram.service';
import { ChannelSender } from './interfaces/channel-sender.interface';

@Injectable()
export class MessagingService {
  private readonly logger = new Logger('Messaging');
  private readonly senders: Record<string, ChannelSender>;

  constructor(
    private prisma: PrismaService,
    private evolution: EvolutionApiService,
    private telegram: TelegramService,
  ) {
    this.senders = {
      whatsapp: this.evolution,
      telegram: this.telegram,
    };
  }

  async send(channel: string, target: string, message: string): Promise<{ messageId: string }> {
    const sender = this.senders[channel];
    if (!sender) throw new Error(`Unsupported channel: ${channel}`);

    // Upsert contact
    const channelType = channel as any;
    const isGroup = target.endsWith('@g.us') || false;
    const contact = await this.prisma.messagingContact.upsert({
      where: { channelType_remoteId: { channelType, remoteId: target } },
      create: { channelType, remoteId: target, isGroup },
      update: { lastSeenAt: new Date() },
    });

    // Send message
    try {
      const result = await sender.send(target, message);

      // Log outbound message
      await this.prisma.messageLog.create({
        data: {
          contactId: contact.id,
          direction: 'outbound',
          content: message.substring(0, 4000),
          messageId: result.messageId,
          status: 'sent',
        },
      });

      return result;
    } catch (err) {
      // Log failed message
      await this.prisma.messageLog.create({
        data: {
          contactId: contact.id,
          direction: 'outbound',
          content: message.substring(0, 4000),
          status: 'failed',
          errorMessage: err.message,
        },
      });
      throw err;
    }
  }

  async logInbound(channel: string, remoteId: string, text: string, displayName?: string, messageId?: string) {
    const channelType = channel as any;
    const isGroup = remoteId.endsWith('@g.us');

    const contact = await this.prisma.messagingContact.upsert({
      where: { channelType_remoteId: { channelType, remoteId } },
      create: { channelType, remoteId, isGroup, displayName },
      update: { lastSeenAt: new Date(), displayName: displayName || undefined },
    });

    // Auto-link to person if not linked yet
    if (!contact.personId && displayName && !isGroup) {
      try {
        const person = await this.prisma.person.findFirst({
          where: { name: { contains: displayName.split(' ')[0], mode: 'insensitive' } },
        });
        if (person) {
          await this.prisma.messagingContact.update({
            where: { id: contact.id },
            data: { personId: person.id },
          });
        }
      } catch {}
    }

    await this.prisma.messageLog.create({
      data: {
        contactId: contact.id,
        direction: 'inbound',
        content: text.substring(0, 4000),
        messageId,
        status: 'received',
      },
    });

    return contact;
  }

  async healthCheck(): Promise<Record<string, boolean>> {
    const results: Record<string, boolean> = {};
    for (const [name, sender] of Object.entries(this.senders)) {
      results[name] = await sender.healthCheck();
    }
    return results;
  }
}
