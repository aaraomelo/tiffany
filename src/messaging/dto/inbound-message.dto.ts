export interface InboundMessage {
  channelType: 'whatsapp' | 'telegram';
  remoteId: string;
  senderPhone: string;
  displayName: string;
  text: string;
  isGroup: boolean;
  groupName?: string;
  quotedMessageId?: string;
  mediaType?: string;
  timestamp: Date;
}
