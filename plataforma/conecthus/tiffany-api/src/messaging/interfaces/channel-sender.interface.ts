export interface ChannelSender {
  send(remoteId: string, text: string): Promise<{ messageId: string }>;
  sendMedia(remoteId: string, url: string, caption: string): Promise<{ messageId: string }>;
  healthCheck(): Promise<boolean>;
}
