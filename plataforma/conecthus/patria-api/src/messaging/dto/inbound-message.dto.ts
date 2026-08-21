export interface InboundMessage {
  channelType: 'whatsapp' | 'telegram';
  remoteId: string;
  senderPhone: string;
  displayName: string;
  text: string;
  isGroup: boolean;
  groupContext?: string;
  groupName?: string;
  quotedMessageId?: string;
  mediaType?: string;
  timestamp: Date;
}
