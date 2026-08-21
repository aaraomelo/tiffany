import {
  Body,
  Controller,
  Delete,
  Get,
  Param,
  ParseUUIDPipe,
  Post,
  Put,
} from '@nestjs/common';
import { UserRole } from '@prisma/client';
import { IsArray, IsBoolean, IsEnum, IsOptional, IsString } from 'class-validator';
import { AssistantConfigService } from './assistant-config.service';
import { AssistantLlmService } from './assistant-llm.service';
import { AssistantMemoryService } from './assistant-memory.service';
import { AssistantProfileService } from './assistant-profile.service';

class UpdateConfigDto {
  @IsOptional() @IsString() llmProvider?: string;
  @IsOptional() @IsString() model?: string;
  @IsOptional() @IsString() apiKey?: string;
  @IsOptional() @IsString() soulPrompt?: string;
  @IsOptional() @IsBoolean() active?: boolean;
}

class UpdateProfileDto {
  @IsOptional() @IsString() name?: string;
  @IsOptional() @IsString() systemPrompt?: string;
  @IsOptional() @IsArray() allowedTools?: string[];
  @IsOptional() @IsString() memoryAccess?: string;
  @IsOptional() @IsBoolean() active?: boolean;
}

class ChatDto {
  @IsOptional() @IsString() conversationId?: string;
  @IsString() text!: string;
}

class SaveMemoryDto {
  @IsString() title!: string;
  @IsString() content!: string;
  @IsString() category!: string;
  @IsOptional() @IsString() priority?: 'long_term' | 'short_term';
  @IsOptional() @IsString() visibility?: 'tenant_global' | 'private';
}

@Controller('assistant')
export class AssistantController {
  constructor(
    private readonly config: AssistantConfigService,
    private readonly profiles: AssistantProfileService,
    private readonly llm: AssistantLlmService,
    private readonly memory: AssistantMemoryService,
  ) {}

  // ---------- config ----------
  @Get('config')
  getConfig() {
    return this.config.getView();
  }

  @Put('config')
  updateConfig(@Body() dto: UpdateConfigDto) {
    return this.config.update(dto);
  }

  // ---------- profiles ----------
  @Post('profiles/bootstrap')
  bootstrap() {
    return this.profiles.bootstrap();
  }

  @Get('profiles')
  listProfiles() {
    return this.profiles.list();
  }

  @Get('profiles/:role')
  findProfile(@Param('role') role: UserRole) {
    return this.profiles.findByRole(role);
  }

  @Put('profiles/:role')
  updateProfile(@Param('role') role: UserRole, @Body() dto: UpdateProfileDto) {
    return this.profiles.update(role, dto);
  }

  @Post('profiles/:role/reset')
  resetProfile(@Param('role') role: UserRole) {
    return this.profiles.resetToDefault(role);
  }

  // ---------- chat ----------
  @Post('chat')
  chat(@Body() dto: ChatDto) {
    return this.llm.chat(dto);
  }

  @Get('conversations')
  conversations() {
    return this.llm.listConversations();
  }

  @Get('conversations/:id/messages')
  messages(@Param('id', ParseUUIDPipe) id: string) {
    return this.llm.listMessages(id);
  }

  @Delete('conversations/:id')
  deleteConversation(@Param('id', ParseUUIDPipe) id: string) {
    return this.llm.deleteConversation(id);
  }

  // ---------- memory ----------
  @Get('memories')
  listMemories() {
    return this.memory.list();
  }

  @Post('memories')
  saveMemory(@Body() dto: SaveMemoryDto) {
    return this.memory.save(dto);
  }

  @Delete('memories/:target')
  forgetMemory(@Param('target') target: string) {
    return this.memory.forget(target);
  }
}
