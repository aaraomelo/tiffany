export const REPO_DIRS: Record<string, string> = {
  landpage: '/root/landpage-repo',
  'patria-api': '/root/patria-api-repo',
  'patria-app': '/root/patria-app-repo',
};

export const BRANCH_MAP: Record<string, string> = {
  dev: 'develop',
  homolog: 'homolog',
  prod: 'main',
};

export const ENV_URLS: Record<string, Record<string, string>> = {
  dev: { landpage: 'https://dev.patriatechnology.com', 'patria-api': 'https://patria.dev.patriatechnology.com/api', 'patria-app': 'https://patria.dev.patriatechnology.com' },
  homolog: { landpage: 'https://homolog.patriatechnology.com', 'patria-api': 'https://patria.homolog.patriatechnology.com/api', 'patria-app': 'https://patria.homolog.patriatechnology.com' },
  prod: { landpage: 'https://patriatechnology.com', 'patria-api': 'https://patria.patriatechnology.com/api', 'patria-app': 'https://patria.patriatechnology.com' },
};

export const PROJECT_URLS: Record<string, string> = {
  landpage: 'https://patriatechnology.com',
  'patria-api': 'https://patria.patriatechnology.com/api',
  'patria-app': 'https://patria.patriatechnology.com',
};

export const PORT_MAP: Record<string, number> = { dev: 8082, homolog: 8081, prod: 8080 };

export const REPO_PRIORITY: Record<string, number> = { 'patria-api': 0, 'patria-app': 1, landpage: 2 };

export const DEFAULT_CHANNEL = 'whatsapp';
export const DEFAULT_TARGET = '+5511977808883';
export const WHATSAPP_GROUP_ID = '120363426492498460@g.us';
export const GROUP_TARGETS: Record<string, string> = { group: WHATSAPP_GROUP_ID, diretoria: WHATSAPP_GROUP_ID };

export const POLL_INTERVAL = 30_000;
export const SH_OPTS = { stdio: 'pipe' as const, timeout: 30_000, shell: '/bin/sh' };
export const SH_LONG = { stdio: 'pipe' as const, timeout: 60_000, shell: '/bin/sh' };
