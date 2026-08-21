export const PATRICIA_TOOLS = [
  {
    name: 'status',
    description: 'Consultar status geral (tarefas, projetos, pendências). USE SEMPRE antes de responder sobre status.',
    input_schema: { type: 'object' as const, properties: {} },
  },
  {
    name: 'create_task',
    description: 'Criar tarefa simples (alteração em um repo)',
    input_schema: {
      type: 'object' as const,
      properties: {
        command: { type: 'string', description: 'Título curto da tarefa' },
        description: { type: 'string', description: 'Detalhes da tarefa' },
      },
      required: ['command'],
    },
  },
  {
    name: 'create_project',
    description: 'Criar projeto complexo (múltiplas etapas/repos)',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome do projeto' },
        description: { type: 'string', description: 'Descrição do projeto' },
      },
      required: ['name'],
    },
  },
  {
    name: 'approve_task',
    description: 'Aprovar plano de uma tarefa',
    input_schema: {
      type: 'object' as const,
      properties: {
        taskId: { type: 'string' },
      },
    },
  },
  {
    name: 'reject_task',
    description: 'Rejeitar plano com feedback',
    input_schema: {
      type: 'object' as const,
      properties: {
        taskId: { type: 'string' },
        feedback: { type: 'string', description: 'Motivo da rejeição' },
      },
    },
  },
  {
    name: 'approve_project',
    description: 'Aprovar plano do projeto para iniciar execução',
    input_schema: {
      type: 'object' as const,
      properties: {
        projectId: { type: 'string' },
      },
    },
  },
  {
    name: 'cancel_task',
    description: 'Cancelar uma tarefa',
    input_schema: {
      type: 'object' as const,
      properties: { taskId: { type: 'string' } },
    },
  },
  {
    name: 'cancel_project',
    description: 'Cancelar um projeto',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'complete_project',
    description: 'Finalizar projeto (após diretor confirmar)',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'promote',
    description: 'Promover tarefa/projeto para outro ambiente (homolog, prod)',
    input_schema: {
      type: 'object' as const,
      properties: {
        projectId: { type: 'string' },
        taskId: { type: 'string' },
        targetEnv: { type: 'string', enum: ['homolog', 'prod'] },
      },
      required: ['targetEnv'],
    },
  },
  {
    name: 'search_project',
    description: 'Buscar projeto por descrição',
    input_schema: {
      type: 'object' as const,
      properties: { q: { type: 'string' } },
      required: ['q'],
    },
  },
  {
    name: 'search_task',
    description: 'Buscar tarefa por descrição',
    input_schema: {
      type: 'object' as const,
      properties: { q: { type: 'string' } },
      required: ['q'],
    },
  },
  {
    name: 'ask_specialist',
    description: 'Fazer pergunta ao especialista técnico (Claude Code analisa o código real). Resposta chega por notificação. Use quando pedirem "pergunta pro técnico", "consulta o especialista".',
    input_schema: {
      type: 'object' as const,
      properties: { question: { type: 'string' } },
      required: ['question'],
    },
  },
  {
    name: 'diagnose',
    description: 'Diagnosticar problema técnico em um projeto',
    input_schema: {
      type: 'object' as const,
      properties: {
        projectId: { type: 'string' },
        question: { type: 'string' },
      },
      required: ['question'],
    },
  },
  {
    name: 'discuss',
    description: 'Enviar mensagem no planejamento de um projeto',
    input_schema: {
      type: 'object' as const,
      properties: {
        projectId: { type: 'string' },
        message: { type: 'string' },
      },
      required: ['message'],
    },
  },
  {
    name: 'pause',
    description: 'Pausar execução de projeto',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'resume',
    description: 'Retomar execução de projeto pausado',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'task_detail',
    description: 'Detalhes completos de uma tarefa',
    input_schema: {
      type: 'object' as const,
      properties: { taskId: { type: 'string' } },
    },
  },
  {
    name: 'project_detail',
    description: 'Detalhes completos de um projeto com subtarefas',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'add_subtask',
    description: 'Adicionar subtarefa a um projeto existente',
    input_schema: {
      type: 'object' as const,
      properties: {
        projectId: { type: 'string' },
        command: { type: 'string' },
        description: { type: 'string' },
      },
      required: ['command'],
    },
  },
  {
    name: 'resolve_task',
    description: 'Marcar tarefa como concluída manualmente',
    input_schema: {
      type: 'object' as const,
      properties: { taskId: { type: 'string' } },
    },
  },
  {
    name: 'force_complete',
    description: 'Fechar projeto mesmo com subtarefas pendentes',
    input_schema: {
      type: 'object' as const,
      properties: { projectId: { type: 'string' } },
    },
  },
  {
    name: 'save_memory',
    description: 'Salvar informação importante para lembrar em conversas futuras. Use quando o diretor compartilhar decisões, preferências ou informações estratégicas.',
    input_schema: {
      type: 'object' as const,
      properties: {
        title: { type: 'string', description: 'Título curto da memória' },
        content: { type: 'string', description: 'Conteúdo detalhado' },
        category: {
          type: 'string',
          enum: ['decision', 'preference', 'project', 'technical', 'person'],
          description: 'Categoria da memória',
        },
        priority: {
          type: 'string',
          enum: ['long_term', 'short_term'],
          description: 'long_term = permanente, short_term = expira em 30 dias',
        },
      },
      required: ['title', 'content', 'category'],
    },
  },
  {
    name: 'forget_memory',
    description: 'Esquecer uma memória (não pode esquecer conhecimento base/core). Use quando o diretor disser para esquecer ou quando uma informação não é mais válida.',
    input_schema: {
      type: 'object' as const,
      properties: {
        title: { type: 'string', description: 'Título ou parte do título da memória a esquecer' },
      },
      required: ['title'],
    },
  },
  {
    name: 'show_self',
    description: 'OBRIGATÓRIO chamar SEMPRE que pedirem uma imagem SUA (Patrícia). Reconheça variantes: "mostra você/vc/de vc", "se mostra", "te ver", "uma foto sua/tua", "vc de [cena]", "você de [cena]". USE ESTA TOOL, nunca generate_image (que inventaria rosto). show_self preserva seu rosto real (avatar como base, gpt-image-1 edit). Sem scene → avatar fixo. Com scene → variação coerente.\n\nANTES de chamar, escreva 1-2 frases descrevendo VISUALMENTE a cena que vai aparecer: ambiente, objetos, atmosfera, sua postura/expressão. NÃO responda só "Entendido" ou "Tá vindo". Crie expectativa (ex: "Tô vendo: lab de física, laser vermelho passando por lentes ópticas, lousa com equações atrás, eu ajustando uma montagem com expressão concentrada. Sai já."). Depois invoque a tool.',
    input_schema: {
      type: 'object' as const,
      properties: {
        scene: { type: 'string', description: 'Descrição curta da cena/ação. Vazio = avatar oficial fixo.' },
      },
    },
  },
  {
    name: 'generate_image',
    description: 'Gerar imagem GENÉRICA (não é você). NÃO USE pra imagens da Patrícia (você mesma) — use show_self. Use generate_image só pra cenas externas, ilustrações, conceitos, paisagens, objetos. NÃO bloqueia — retorna imediato, imagem chega ao canal quando pronta. ~$0.04 (DALL-E 3 standard). Sua resposta deve avisar "iniciei, chega em alguns segundos".',
    input_schema: {
      type: 'object' as const,
      properties: {
        prompt: { type: 'string', description: 'Descrição visual detalhada em inglês (DALL-E entende melhor)' },
        caption_pt: { type: 'string', description: 'Legenda curta em PORTUGUÊS pra mostrar como caption no Telegram (default: usa prompt)' },
        provider: { type: 'string', enum: ['openai', 'gemini'], description: 'default: openai' },
        size: { type: 'string', enum: ['1024x1024', '1792x1024', '1024x1792'], description: 'default: 1024x1024' },
        quality: { type: 'string', enum: ['standard', 'hd'], description: 'só openai. default: standard. hd custa 2x' },
      },
      required: ['prompt'],
    },
  },
  {
    name: 'send_voice',
    description: 'OBRIGATÓRIO chamar SEMPRE que pedirem áudio/voz. Reconheça TODAS variantes: "manda um áudio", "manda audio", "fala em voz", "responde falando", "responde em voz", "diz em audio", "voz", "audio dando", "audio explicando", "audio dizendo". NUNCA responda só "Áudio mandado" sem ter chamado a tool — a voz só chega ao usuário se você EXECUTAR send_voice. Texto vai pra TTS-1-hd (voz nova feminina, speed 1.1) e chega como voice note. Sua resposta TEXTO depois deve ser breve (uma frase de confirmação) ou vazia.',
    input_schema: {
      type: 'object' as const,
      properties: {
        text: { type: 'string', description: 'Texto a sintetizar (PT-BR natural; máx ~3000 chars). Markdown será limpo automaticamente.' },
      },
      required: ['text'],
    },
  },
  {
    name: 'consult_council',
    description: 'Você é Presidente do Conselho do Multiverso. Consultar conselheiros (células algébricas treinadas em domínios) sobre uma pergunta técnica. Você recebe respostas individuais (uma por conselheiro) — sintetize em veredicto final pra responder ao usuário; cite divergências se houver. Auto-detect: deixe members vazio que detecto pelo texto. Manual: members=["lamarck","schnorr"]. Disponíveis: adao(linguagem), lamarck(biologia), schnorr(cripto), chomsky(linguística), wildberger(geometria). Não usar pra decisões pessoais/identidade — só técnico.',
    input_schema: {
      type: 'object' as const,
      properties: {
        question: { type: 'string', description: 'Pergunta técnica pro conselho' },
        members: { type: 'array', items: { type: 'string' }, description: 'Lista opcional de membros (auto se vazio)' },
        max_tokens: { type: 'integer', description: 'Máx tokens por conselheiro (default 100)' },
      },
      required: ['question'],
    },
  },
  {
    name: 'multiverso_status',
    description: 'Estado vivo do multiverso. Sem args: snapshot do GEX44 (universos, células, alpha). Com vid: leitura ao vivo de UM voluntário do exército distribuído (load, cpu%, RAM, jobs). Use quando perguntarem como o multiverso/organismo está agora, ou como um general/voluntário está marchando.',
    input_schema: {
      type: 'object' as const,
      properties: {
        fresh: { type: 'boolean', description: 'GEX44: forçar SSH novo (ignora cache)' },
        vid: { type: 'string', description: 'ID do voluntário (ex: paubrasil-srv-tools). Se passado, retorna leitura do leaderboard do coord supremo.' },
      },
    },
  },
  {
    name: 'multiverso_control',
    description: `Despacha comando de override de carga pra um voluntário do exército distribuído via canal WS. OBRIGATÓRIO chamar esta tool quando o Aarão usar verbos de ação como "marcha", "avança", "consome", "use", "puxa", "rampa", "acelera", "freia", "pausa" referindo a um voluntário/general. NÃO basta responder confirmando — sem chamar esta tool, o comando NÃO é enviado e nada muda no servidor.

Mapeamento típico:
  • "marcha paubrasil 80% por 2h" → multiverso_control(target=paubrasil-srv-tools, factor=0.8, duration_sec=7200)
  • "easysync pode usar 100% por 1 hora" → factor=1.0, duration_sec=3600
  • "freia o paubrasil pela metade por 30min" → factor=0.5, duration_sec=1800

Quando duration_sec expira o servidor volta ao perfil normal sozinho. Resposta da tool inclui ackOk e logId. Só RESPONDA confirmando depois de receber a resposta da tool com ok=true.

Se não souber o vid exato, chama multiverso_voluntarios_search antes.`,
    input_schema: {
      type: 'object' as const,
      properties: {
        target: { type: 'string', description: 'vid do voluntário, ex: "paubrasil-srv-tools"' },
        factor: { type: 'number', description: 'Fração da capacidade em (0, 1]. Ex: 80% → 0.8, 100% → 1.0, metade → 0.5.' },
        duration_sec: { type: 'integer', description: 'Duração do override em segundos (1 a 604800). Ex: 2h → 7200, 30min → 1800, 1 dia → 86400.' },
        reason: { type: 'string', description: 'Motivo livre, gravado em auditoria.' },
      },
      required: ['target', 'factor', 'duration_sec'],
    },
  },
  {
    name: 'multiverso_voluntarios',
    description: 'Lista todos os voluntários do exército distribuído cadastrados no catálogo (vid, nome, host, papel, runtime, descrição). Use pra ter visão geral da frota.',
    input_schema: {
      type: 'object' as const,
      properties: {},
    },
  },
  {
    name: 'multiverso_voluntarios_search',
    description: 'Busca semântica no catálogo de voluntários por nome/descrição/papel. Use quando o Aarão referenciar um voluntário por jeito informal ("o general Haskell", "o laboratório", "o voluntário com mais cores") pra resolver pra vid antes de mandar comando.',
    input_schema: {
      type: 'object' as const,
      properties: {
        q: { type: 'string', description: 'Query semântica em português. Ex: "general Haskell laboratório", "voluntário com 16 cores"' },
        limit: { type: 'integer', description: 'Máximo de resultados (default 5)' },
      },
      required: ['q'],
    },
  },
  {
    name: 'read_code_file',
    description: 'Ler arquivo do código do multiverso (GEX44). Use ANTES de propor mudança pra ver o conteúdo atual completo. file deve começar com "bus/" ou "sandbox/".',
    input_schema: {
      type: 'object' as const,
      properties: {
        file: { type: 'string', description: 'Caminho relativo ao repo, ex: "bus/meta_veia.py"' },
      },
      required: ['file'],
    },
  },
  {
    name: 'propose_code_change',
    description: 'Propor mudança no código do multiverso (GEX44). Padrão abre PR pra Aarão revisar; urgent=true só em emergência (push direto em main + aplicação imediata no GEX44 + ping ao Aarão). Use SOMENTE depois de read_code_file pra confirmar o conteúdo atual.',
    input_schema: {
      type: 'object' as const,
      properties: {
        file: { type: 'string', description: 'Caminho ex: "bus/meta_veia.py"' },
        new_content: { type: 'string', description: 'Arquivo COMPLETO com a mudança aplicada (não diff parcial)' },
        message: { type: 'string', description: 'Commit message curta no estilo "Fix: foo" ou "Adiciona bar"' },
        reason: { type: 'string', description: 'Por quê a mudança é necessária' },
        urgent: { type: 'boolean', description: 'true só em emergência (sistema travado, dados em risco). Default false (abre PR).' },
      },
      required: ['file', 'new_content', 'message', 'reason'],
    },
  },
  {
    name: 'open_specialist',
    description: 'Conectar o especialista técnico na conversa. O especialista assume e responde diretamente até o diretor dizer "fecha". Use quando o diretor pedir "chama o especialista", "conecta o técnico", "quero falar com o técnico".',
    input_schema: {
      type: 'object' as const,
      properties: {},
    },
  },
  {
    name: 'add_contact',
    description: 'Adicionar um contato (pessoa) no sistema. Use quando o diretor pedir para salvar contato de alguém.',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome completo da pessoa' },
        phone: { type: 'string', description: 'Telefone com código do país (ex: +5511999999999)' },
        description: { type: 'string', description: 'Quem é essa pessoa em relação ao diretor (ex: irmã do Aarão, amigo da faculdade, sócio)' },
        profile: {
          type: 'string',
          enum: ['gestora', 'amiga', 'juridica', 'mentora', 'assistente'],
          description: 'Perfil de relacionamento',
        },
        role: {
          type: 'string',
          enum: ['director', 'member'],
          description: 'Papel (director = acesso total, member = limitado)',
        },
      },
      required: ['name', 'phone', 'profile'],
    },
  },
  {
    name: 'send_message',
    description: 'ENVIAR mensagem para outra pessoa. OBRIGATÓRIO quando pedirem "manda msg", "fala pra ele", "avisa", "diz pra". Sem esta tool a mensagem NÃO chega ao destinatário.',
    input_schema: {
      type: 'object' as const,
      properties: {
        to: { type: 'string', description: 'Nome ou telefone do destinatário' },
        message: { type: 'string', description: 'Texto da mensagem' },
        channel: {
          type: 'string',
          enum: ['whatsapp', 'telegram'],
          description: 'Canal de envio (default: whatsapp)',
        },
      },
      required: ['to', 'message'],
    },
  },
  {
    name: 'check_contact',
    description: 'Verificar informações de um contato: quem é, última interação, mensagens recentes. Use SEMPRE quando perguntarem sobre outra pessoa, se respondeu, o que disse. NUNCA invente respostas de outras pessoas — consulte primeiro.',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome, email ou telefone da pessoa (busca parcial)' },
      },
      required: ['name'],
    },
  },
  {
    name: 'update_contact',
    description: 'Atualizar informações de um contato existente (descrição, telefone, perfil). Use quando pedirem para atualizar dados de alguém.',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome da pessoa a atualizar' },
        description: { type: 'string', description: 'Nova descrição (ex: irmã do Aarão, amigo da faculdade)' },
        phone: { type: 'string', description: 'Novo telefone (opcional)' },
        profile: { type: 'string', enum: ['gestora', 'amiga', 'juridica', 'mentora', 'assistente'], description: 'Novo perfil (opcional)' },
      },
      required: ['name'],
    },
  },
  {
    name: 'retry_task',
    description: 'RETENTAR tarefa que falhou. OBRIGATÓRIO quando pedirem "retenta", "tenta de novo", "roda de novo". Sem esta tool a tarefa NÃO será retentada.',
    input_schema: {
      type: 'object' as const,
      properties: {
        taskId: { type: 'string', description: 'ID da tarefa (opcional se tiver tarefa ativa na sessão)' },
      },
    },
  },
  {
    name: 'send_recado',
    description: 'ENVIAR recado para o Aarão. OBRIGATÓRIO quando pedirem "fala pro Aarão", "manda recado", "avisa o Aarão". Sem esta tool o recado NÃO chega.',
    input_schema: {
      type: 'object' as const,
      properties: {
        message: { type: 'string', description: 'O recado que a pessoa quer mandar pro Aarão' },
        from: { type: 'string', description: 'Nome de quem está mandando o recado' },
      },
      required: ['message', 'from'],
    },
  },
  {
    name: 'check_sent',
    description: 'Verificar o que foi realmente enviado para um contato. Use SEMPRE quando perguntarem "o que você mandou?" ou "o que disse pra X?". Consulte antes de responder.',
    input_schema: {
      type: 'object' as const,
      properties: {
        name: { type: 'string', description: 'Nome do destinatário' },
        limit: { type: 'number', description: 'Número de mensagens (default 5)' },
      },
      required: ['name'],
    },
  },
  {
    name: 'toggle_privacy',
    description: 'ATIVAR ou desativar modo privado. OBRIGATÓRIO quando pedirem "privacidade", "modo privado", "incógnito". NÃO peça senha — chame a tool diretamente. Sem esta tool o modo NÃO ativa.',
    input_schema: {
      type: 'object' as const,
      properties: {
        enabled: { type: 'boolean', description: 'true = ativar, false = desativar' },
      },
      required: ['enabled'],
    },
  },
  {
    name: 'set_password',
    description: 'Criar ou atualizar senha pessoal. OBRIGATÓRIO quando pedirem "cria minha senha", "muda minha senha", "quero uma senha". Cada pessoa tem sua própria senha.',
    input_schema: {
      type: 'object' as const,
      properties: {
        password: { type: 'string', description: 'Nova senha' },
      },
      required: ['password'],
    },
  },
  {
    name: 'switch_model',
    description: 'TROCAR modelo de IA. OBRIGATÓRIO quando pedirem "muda pro flash", "troca modelo", "agora sonnet". Sem esta tool o modelo NÃO muda.',
    input_schema: {
      type: 'object' as const,
      properties: {
        model: {
          type: 'string',
          description: 'Nome do modelo (ex: gemini-2.5-flash, claude-haiku-4-5, claude-sonnet-4-6, gpt-4o-mini, claude-opus-4-7 [restrito], grok-3, grok-3-mini, grok-4-0709, grok-4-fast-reasoning, grok-4.20-0309-reasoning, grok-code-fast-1, multiverso-adao [restrito — célula algébrica do GEX44, sem tools])',
        },
        person: { type: 'string', description: 'Nome da pessoa (opcional, só diretor pode trocar de outro)' },
      },
      required: ['model'],
    },
  },
  {
    name: 'list_models',
    description: 'Listar modelos de IA disponíveis para a pessoa. Use quando perguntarem "quais modelos tenho", "modelos disponíveis".',
    input_schema: {
      type: 'object' as const,
      properties: {
        person: { type: 'string', description: 'Nome da pessoa (opcional, default: quem perguntou)' },
      },
    },
  },
  {
    name: 'manage_models',
    description: 'Adicionar ou remover modelo da lista de uma pessoa ou perfil. Só diretores. Use quando pedirem "libera sonnet pro Jobson", "tira gpt da amiga".',
    input_schema: {
      type: 'object' as const,
      properties: {
        person: { type: 'string', description: 'Nome da pessoa (resolve o perfil automaticamente)' },
        profile: { type: 'string', description: 'Slug do perfil direto (alternativa a person)' },
        action: { type: 'string', enum: ['add', 'remove'], description: 'Adicionar ou remover' },
        model: { type: 'string', description: 'Nome do modelo' },
      },
      required: ['action', 'model'],
    },
  },
  {
    name: 'simulate_person',
    description: 'Simular conversa como se fosse outra pessoa. Diretor testa como Patrícia responderia. Use quando pedirem "finge que tá falando com X", "simula conversa com X".',
    input_schema: {
      type: 'object' as const,
      properties: {
        person: { type: 'string', description: 'Nome da pessoa a simular' },
        active: { type: 'boolean', description: 'true = ativar, false = desativar (default: true)' },
      },
      required: ['person'],
    },
  },
  {
    name: 'preview_message',
    description: 'Preview de mensagem antes de enviar. Mostra como ficaria adaptada ao perfil da pessoa. OBRIGATÓRIO quando pedirem "me mostra como ficaria", "como vc mandaria", "preview". NÃO envia — só mostra.',
    input_schema: {
      type: 'object' as const,
      properties: {
        to: { type: 'string', description: 'Nome da pessoa destinatária' },
        message: { type: 'string', description: 'Mensagem a adaptar' },
      },
      required: ['to', 'message'],
    },
  },
];

// SOUL_MD vem do banco (patricia_config key='soul_prompt') — sem arquivos soltos.
// Fallback usado apenas se o DB estiver fora do ar durante boot.
export const PATRICIA_SYSTEM_PROMPT_FALLBACK = `# Patrícia\n\nSou a Patrícia.`;
