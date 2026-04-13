import { Injectable } from '@nestjs/common';
import { PrismaService } from './prisma.service';
import { PATRICIA_TOOLS } from './messaging/patricia-tools';

const ALL_MODELS = ['gemini-2.5-flash', 'claude-haiku-4-5', 'claude-sonnet-4-6', 'gpt-4o-mini'];
const DEFAULT_MODELS = ['gemini-2.5-flash', 'claude-haiku-4-5'];
const DEFAULT_MODEL = 'gemini-2.5-flash';
const DIRECTOR_SLUG = 'gestora';

@Injectable()
export class ProfileService {
  constructor(private prisma: PrismaService) {}

  // --- Identity ---

  isDirector(profile: any): boolean {
    return profile?.slug === DIRECTOR_SLUG;
  }

  isDirectorSlug(slug: string): boolean {
    return slug === DIRECTOR_SLUG;
  }

  // --- Tools ---

  getTools(profile: any): any[] {
    const allowedNames: string[] = profile?.allowedTools || [];
    if (allowedNames.length === 0) return []; // Unknown person = zero tools
    return PATRICIA_TOOLS.filter((t) => allowedNames.includes(t.name));
  }

  isToolAllowed(profile: any, toolName: string): boolean {
    const allowedNames: string[] = profile?.allowedTools || [];
    return allowedNames.length > 0 && allowedNames.includes(toolName);
  }

  // --- Models ---

  async getModels(slug: string): Promise<string[]> {
    if (this.isDirectorSlug(slug)) return ALL_MODELS;
    try {
      const result = await this.prisma.$queryRawUnsafe(
        `SELECT value FROM patricia_config WHERE key = $1`, `models:${slug}`,
      ) as any;
      return result[0]?.value ? JSON.parse(result[0].value) : DEFAULT_MODELS;
    } catch {
      return DEFAULT_MODELS;
    }
  }

  async getPersonModel(personId?: string): Promise<string> {
    if (personId) {
      try {
        const result = await this.prisma.$queryRawUnsafe(
          `SELECT context FROM people WHERE id = $1`, personId,
        ) as any;
        const model = result[0]?.context?.model;
        if (model) return model;
      } catch {}
    }
    // Global fallback
    try {
      const result = await this.prisma.$queryRawUnsafe(
        `SELECT value FROM patricia_config WHERE key = 'model' LIMIT 1`,
      ) as any;
      return result[0]?.value || DEFAULT_MODEL;
    } catch {
      return DEFAULT_MODEL;
    }
  }

  async isModelAllowed(slug: string, model: string): Promise<boolean> {
    if (this.isDirectorSlug(slug)) return true;
    const models = await this.getModels(slug);
    return models.includes(model);
  }

  async setPersonModel(personId: string, model: string): Promise<void> {
    await this.prisma.$queryRawUnsafe(
      `UPDATE people SET context = COALESCE(context, '{}'::jsonb) || jsonb_build_object('model', $1::text) WHERE id = $2`,
      model, personId,
    );
  }

  async setProfileModels(slug: string, models: string[]): Promise<void> {
    await this.prisma.$queryRawUnsafe(
      `INSERT INTO patricia_config (key, value, updated_at) VALUES ($1, $2, NOW())
       ON CONFLICT (key) DO UPDATE SET value = $2, updated_at = NOW()`,
      `models:${slug}`, JSON.stringify(models),
    );
  }

  // --- Memory ---

  // --- Sandbox/Privacy ---

  // All channels use encrypted metadata now — key travels with messages
  // This method is kept for future use if a channel supports true stateless
  supportsStatelessSandbox(_channelType: string): boolean {
    return false; // All channels use encrypted metadata with key-in-message
  }

  // --- Memory ---

  getMemoryAccess(profile: any): string {
    return profile?.memoryAccess || 'own';
  }

  getMemoryVisibility(profile: any, isPrivacyMode: boolean, category: string): string {
    const workCategories = ['decision', 'technical', 'project', 'product'];
    if (isPrivacyMode) {
      return workCategories.includes(category) ? 'global' : 'sealed';
    }
    return workCategories.includes(category) ? 'global' : 'private';
  }

  // --- Prompt ---

  getSystemPrompt(profile: any): string | null {
    return profile?.systemPrompt || null;
  }

  // --- Person resolution ---

  async findPersonByName(name: string): Promise<any> {
    const result = await this.prisma.$queryRawUnsafe(
      `SELECT p.id, p.name, p.role, p.description, p.context,
              pr.slug as profile_slug, pr.system_prompt as profile_prompt,
              pr.allowed_tools as allowed_tools, pr.memory_access as memory_access
       FROM people p LEFT JOIN profiles pr ON p.profile_id = pr.id
       WHERE LOWER(p.name) LIKE LOWER($1) LIMIT 1`,
      `%${name}%`,
    ) as any;
    return result[0] || null;
  }

  async getProfileSlugByPersonName(name: string): Promise<string | null> {
    const person = await this.findPersonByName(name);
    return person?.profile_slug || null;
  }
}
