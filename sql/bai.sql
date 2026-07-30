-- bai.sql — O ESQUEMA: a Biblioteca de Autorização Isomórfica como banco.
--
-- Fonte do formalismo: broca-so/papers/casl-propagation.tex (a tese-mãe: toda instância é
-- determinada pelo quinteto (κ, δ, Φ, T, Ψ)). Aqui ele não é modelado — é TRANSCRITO, e os
-- teoremas viram RESTRIÇÃO. Bug de autorização é o tipo que não aparece em teste: tem de ser
-- impossível de escrever, não improvável.
--
--   κ = (σ, c, δ, ω, o)   σ escopo · c condição · δ orçamento · ω peso · o origem
--   T ⪯ id   atenuação: delegar só ESTREITA
--   P ⪰ id   propagação, limitada por δ
--   Ψ = Collapse(𝒞_δ, q)  e ⊥ quando não há concessão — fail-closed
--
-- A isomorfia é o ponto: usuário e nó de conhecimento são o MESMO formalismo. Por isso
-- `principal` tem tipo, e não duas tabelas.

PRAGMA foreign_keys = ON;

-- O TENANT. Isto não é infraestrutura: é a fronteira de autorização de fora para dentro.
-- No patria o tenant vem do subdomínio ({alias}.patriatechnology.com -> X-Tenant -> alias),
-- e o USUÁRIO É O TENANT da BAI — o detentor primeiro, de quem tudo o mais deriva.
CREATE TABLE tenant (
  id    TEXT PRIMARY KEY,
  alias TEXT NOT NULL UNIQUE,       -- o subdomínio; é ele que chega no cabeçalho
  nome  TEXT NOT NULL
);

CREATE TABLE principal (
  id     TEXT PRIMARY KEY,
  tenant TEXT NOT NULL REFERENCES tenant(id) ON DELETE CASCADE,
  tipo   TEXT NOT NULL CHECK (tipo IN ('usuario','agente','no')),
  nome   TEXT NOT NULL
);

-- c: a condição. Tabela própria, não coluna — porque o RETÍCULO é compartilhado entre
-- instâncias, e é ele que garante que dois instrumentos não se contradigam (tools/instrumento.c:
-- as partições de leis diferentes refinam-se, nunca se cruzam).
CREATE TABLE condicao (
  id        TEXT PRIMARY KEY,
  descricao TEXT NOT NULL
);

-- a aresta do retículo: ext(filha) ⊆ ext(mae). A reflexiva não se guarda (vem na view).
CREATE TABLE refina (
  filha TEXT NOT NULL REFERENCES condicao(id) ON DELETE CASCADE,
  mae   TEXT NOT NULL REFERENCES condicao(id) ON DELETE CASCADE,
  PRIMARY KEY (filha, mae),
  CHECK (filha <> mae)
);

-- o fecho reflexo-transitivo do retículo. É ponto fixo — e ponto fixo em SQL é WITH RECURSIVE,
-- que é a mesma semântica da contração de tools/bairro.c, aqui na forma declarativa.
CREATE VIEW refina_estrela AS
WITH RECURSIVE r(filha, mae) AS (
    SELECT id, id FROM condicao
  UNION
    SELECT r.filha, x.mae FROM r JOIN refina x ON x.filha = r.mae
)
SELECT filha, mae FROM r;

CREATE TABLE capacidade (
  id       TEXT PRIMARY KEY,
  detentor TEXT NOT NULL REFERENCES principal(id) ON DELETE CASCADE,
  sigma    TEXT NOT NULL,
  condicao TEXT NOT NULL REFERENCES condicao(id),
  delta    INTEGER NOT NULL CHECK (delta >= 0),
  omega    REAL    NOT NULL CHECK (omega >= 0.0 AND omega <= 1.0),
  -- a proveniência é ESTRUTURA, não carimbo: ou a capacidade é intrínseca (raiz), ou aponta
  -- para o pai que a concedeu. O ON DELETE CASCADE é o Teorema da revogação em cascata —
  -- revogar a raiz alcança os netos, e isso só funciona se a cadeia estiver na tabela desde
  -- o primeiro dia. Não se acrescenta depois.
  pai      TEXT REFERENCES capacidade(id) ON DELETE CASCADE,
  raiz     INTEGER NOT NULL DEFAULT 0 CHECK (raiz IN (0,1)),
  origem   TEXT NOT NULL,
  -- NÃO-ESCALADA, primeira metade: não existe κ sem origem. Sem corpus não há κ.
  CHECK ( (raiz = 1 AND pai IS NULL) OR (raiz = 0 AND pai IS NOT NULL) )
);

CREATE INDEX cap_por_detentor ON capacidade(detentor);
CREATE INDEX cap_por_pai      ON capacidade(pai);
CREATE INDEX cap_por_sigma    ON capacidade(sigma);

-- As três coisas que CHECK não enxerga (pedem consulta), na porta de entrada.
CREATE TRIGGER bai_delegacao BEFORE INSERT ON capacidade
WHEN NEW.pai IS NOT NULL
BEGIN
  -- PROPAGAÇÃO LIMITADA: cada aresta decrementa δ, e δ=0 não delega.
  SELECT RAISE(ABORT, 'propagacao limitada: o pai nao tem orcamento, ou o filho nao decrementou')
   WHERE (SELECT delta FROM capacidade WHERE id = NEW.pai) < 1
      OR NEW.delta > (SELECT delta FROM capacidade WHERE id = NEW.pai) - 1;

  -- NÃO-ESCALADA, segunda metade (T ⪯ id): a condição do filho tem de REFINAR a do pai.
  SELECT RAISE(ABORT, 'nao-escalada: a condicao do filho nao refina a do pai')
   WHERE NOT EXISTS (SELECT 1 FROM refina_estrela r
                      WHERE r.filha = NEW.condicao
                        AND r.mae   = (SELECT condicao FROM capacidade WHERE id = NEW.pai));

  -- o peso atenua: ω não cresce ao descer a cadeia.
  SELECT RAISE(ABORT, 'atenuacao: omega do filho nao pode exceder o do pai')
   WHERE NEW.omega > (SELECT omega FROM capacidade WHERE id = NEW.pai);

  -- COMPARTILHAMENTO CONTROLADO (o corolário): atravessar a fronteira do tenant só com
  -- δ=0 — o destinatário exerce, e ninguém abaixo dele herda. É "dar acesso sem propagar".
  -- Sem esta regra, uma delegação legítima dentro de um tenant vaza para o vizinho na
  -- geração seguinte, e o vazamento é invisível: cada aresta, sozinha, parece correta.
  SELECT RAISE(ABORT, 'cross-tenant: atravessar a fronteira exige delta=0')
   WHERE NEW.delta > 0
     AND (SELECT p.tenant FROM principal p WHERE p.id = NEW.detentor)
      <> (SELECT p.tenant FROM principal p JOIN capacidade k ON k.detentor = p.id
           WHERE k.id = NEW.pai);
END;

-- o mesmo na atualização, senão a porta tem dobradiça mas não tem fechadura
CREATE TRIGGER bai_delegacao_upd BEFORE UPDATE ON capacidade
WHEN NEW.pai IS NOT NULL
BEGIN
  SELECT RAISE(ABORT, 'propagacao limitada (update)')
   WHERE NEW.delta > (SELECT delta FROM capacidade WHERE id = NEW.pai) - 1;
  SELECT RAISE(ABORT, 'nao-escalada (update)')
   WHERE NOT EXISTS (SELECT 1 FROM refina_estrela r
                      WHERE r.filha = NEW.condicao
                        AND r.mae   = (SELECT condicao FROM capacidade WHERE id = NEW.pai));
END;

-- a cadeia até a raiz: a proveniência consultável (thm:noesc verificável, não confiado)
CREATE VIEW cadeia AS
WITH RECURSIVE c(id, ancestral, salto) AS (
    SELECT id, id, 0 FROM capacidade
  UNION ALL
    SELECT c.id, k.pai, c.salto + 1
      FROM c JOIN capacidade k ON k.id = c.ancestral
     WHERE k.pai IS NOT NULL
)
SELECT id, ancestral, salto FROM c;

-- o tenant de cada capacidade, para consulta e para isolamento na leitura
CREATE VIEW capacidade_tenant AS
SELECT k.*, p.tenant FROM capacidade k JOIN principal p ON p.id = k.detentor;

-- 𝒞_δ: as vigentes. δ entra na FORMAÇÃO do conjunto, nunca na leitura (def:collapse).
CREATE VIEW vigente AS
SELECT k.* FROM capacidade k
 WHERE EXISTS (SELECT 1 FROM cadeia c JOIN capacidade r ON r.id = c.ancestral
                WHERE c.id = k.id AND r.raiz = 1);

-- Ψ = Collapse(𝒞_δ, q): o argmax de ω entre as vigentes cuja condição alcança a consulta.
-- Zero linhas É a resposta ⊥ — fail-closed. Medido em tatoeba/centro.c §C7: onde o estrito
-- recusa, o relaxado erra mais (58,0% contra 68,5%). Recusar não perde: evita o pedaço ruim.
CREATE VIEW colapso AS
SELECT v.detentor, v.sigma, cond.id AS consulta, v.id AS capacidade, v.omega
  FROM vigente v
  JOIN refina_estrela r ON r.mae = v.condicao
  JOIN condicao cond    ON cond.id = r.filha
 WHERE v.omega = (SELECT MAX(v2.omega) FROM vigente v2
                   JOIN refina_estrela r2 ON r2.mae = v2.condicao
                  WHERE v2.detentor = v.detentor AND v2.sigma = v.sigma AND r2.filha = cond.id);
