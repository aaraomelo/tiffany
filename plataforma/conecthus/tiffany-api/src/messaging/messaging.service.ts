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
    if (!contact.personId && !isGroup) {
      try {
        let person = null;

        // 1. Try by displayName
        if (displayName) {
          person = await this.prisma.person.findFirst({
            where: { name: { contains: displayName.split(' ')[0], mode: 'insensitive' } },
          });
        }

        // 2. If @lid, try to match with existing person who has another contact
        // by finding a person whose other messaging_contact was recently active
        if (!person && remoteId.endsWith('@lid')) {
          // Find persons who have a @s.whatsapp.net contact but no @lid contact yet
          const candidates = await this.prisma.person.findMany({
            where: {
              contacts: {
                some: { channelType: 'whatsapp', remoteId: { endsWith: '@s.whatsapp.net' } },
                none: { remoteId: { endsWith: '@lid' } },
              },
            },
            include: { contacts: true },
          });
          // If only one candidate, it's likely them
          if (candidates.length === 1) {
            person = candidates[0];
          }
        }

        if (person) {
          await this.prisma.messagingContact.update({
            where: { id: contact.id },
            data: { personId: person.id },
          });
        }
      } catch {}
    }

    // Check if privacy mode is active for this conversation
    let logContent = text.substring(0, 4000);
    try {
      const session = await this.prisma.conversationSession.findFirst({
        where: { channel: channelType, target: remoteId },
      });
      if ((session?.metadata as any)?.privacyMode === true) {
        logContent = '[mensagem privada]';
      }
    } catch {}

    await this.prisma.messageLog.create({
      data: {
        contactId: contact.id,
        direction: 'inbound',
        content: logContent,
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
