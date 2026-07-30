-- bai.sql — A BAI COMO BANCO **NÃO-CUSTODIAL**. Não guardamos nada.
--
-- Correção de fundamento (do Aarão): isto não é um banco que protege dados — é um banco que
-- NÃO OS TEM. Como a cristalchain: quem lê é só o usuário, o conteúdo é cifrado
-- intrinsecamente, e o que fica guardado é lixo sem a autorização de quem tem a chave.
-- A versão anterior deste arquivo tinha `sigma TEXT`, `descricao TEXT`, `nome TEXT` — texto
-- claro, custódia. Estava errada na raiz, não na borda.
--
-- Três consequências de desenho, e nenhuma é opcional:
--
--  1. NADA LEGÍVEL. Todo identificador é COMPROMISSO (sha3); todo conteúdo é `cofre`, cifrado
--     pelo usuário (a cifra do gato, A_n^k mod N, bijeção com recuperação exata —
--     broca-so/linguagem/cifra_de_cristal.py; é de cristal quando o módulo é primo, porque um
--     corpo não racha: só tem os idempotentes triviais). O servidor NUNCA decifra.
--  2. A RAIZ NÃO SE CONCEDE, PROVA-SE. Autoridade intrínseca é posse (cristalchain/
--     bai_autoridade.py). E a `banda` NÃO é hash do tecido — corrigido depois de ler
--     broca-so/ula/broca_banda.h e neuronio/parabola.py: é
--         w = word_de_bytes(dna) ; k = koch_coordenadas(w, gold(w))
--         banda = sha256(atrator, z_koch_re, z_koch_im, resson_c2, resson_calor,
--                        endereco, orbita_local)
--     O hash é só o último passo; tudo antes é a teoria. A identidade de um cliente é um
--     LUGAR no corpo — a estrela na garrafa de Koch —, não uma cadeia arbitrária.
--  3. REVOGAR É NEGAR, NÃO APAGAR. Num sistema que não tem o dado, apagar não revoga nada —
--     quem já copiou, copiou. Revogação é DENY que DOMINA qualquer grant, e domina para baixo.
--
-- E o que o banco AINDA consegue provar sem ler nada: a cadeia de derivação. Como
-- h_filha = sha3(h_mãe ‖ passo), criar filha exige conhecer a mãe, e subir é impossível
-- (mão única). A não-escalada deixa de ser regra e passa a ser aritmética.

PRAGMA foreign_keys = ON;

-- O TENANT é o usuário, e a sua credencial é a POSSE. `banda` é o hash do tecido; não há
-- coluna de nome, e-mail ou qualquer coisa que identifique gente — isso é dado do usuário.
CREATE TABLE tenant (
  banda TEXT PRIMARY KEY CHECK (length(banda) = 64)   -- ver acima: Koch, não hash cru
);

-- O detentor é um compromisso. Não sabemos quem é; sabemos que a banda o cobre.
CREATE TABLE principal (
  h     TEXT PRIMARY KEY CHECK (length(h) = 64),
  banda TEXT NOT NULL REFERENCES tenant(banda) ON DELETE CASCADE
);

-- κ = (σ, c, δ, ω, o). Aqui: σ, c e ω moram DENTRO do cofre (cifrados); δ e a estrutura
-- ficam fora, porque são o que o banco precisa para verificar sem ler.
CREATE TABLE capacidade (
  h        TEXT PRIMARY KEY CHECK (length(h) = 64),
  detentor TEXT NOT NULL REFERENCES principal(h) ON DELETE CASCADE,
  pai      TEXT REFERENCES capacidade(h) ON DELETE CASCADE,
  raiz     INTEGER NOT NULL DEFAULT 0 CHECK (raiz IN (0,1)),
  delta    INTEGER NOT NULL CHECK (delta >= 0),
  passo    TEXT NOT NULL,          -- o que se compôs com o pai para chegar a esta h
  cofre    BLOB NOT NULL,          -- σ, c, ω cifrados. Lixo sem a chave do usuário.
  -- não existe κ sem origem: ou é raiz provada, ou deriva de um pai
  CHECK ( (raiz = 1 AND pai IS NULL) OR (raiz = 0 AND pai IS NOT NULL) )
);

CREATE INDEX cap_det ON capacidade(detentor);
CREATE INDEX cap_pai ON capacidade(pai);

-- REVOGAÇÃO = DENY. Não se apaga o que não se tem; nega-se, e o deny domina.
CREATE TABLE deny (
  h      TEXT PRIMARY KEY REFERENCES capacidade(h) ON DELETE CASCADE,
  quando TEXT NOT NULL
);

-- A porta. Tudo o que segue é verificável SEM decifrar coisa alguma.
CREATE TRIGGER bai_entrada BEFORE INSERT ON capacidade
BEGIN
  -- a raiz prova-se: h tem de derivar da banda do próprio detentor (posse do tecido)
  SELECT RAISE(ABORT, 'raiz: h nao deriva da banda do detentor — posse nao provada')
   WHERE NEW.raiz = 1
     AND NEW.h <> lower(hex(sha3(
           (SELECT p.banda FROM principal p WHERE p.h = NEW.detentor) || NEW.passo)));

  -- a derivada prova-se: h = sha3(h_pai ‖ passo). Criar filha exige CONHECER a mãe;
  -- e não há caminho de volta. A não-escalada é de mão única, por construção.
  SELECT RAISE(ABORT, 'derivacao: h nao e sha3(pai || passo) — cadeia forjada')
   WHERE NEW.pai IS NOT NULL
     AND NEW.h <> lower(hex(sha3(NEW.pai || NEW.passo)));

  -- propagação limitada: cada aresta gasta δ, e δ=0 não repassa
  SELECT RAISE(ABORT, 'propagacao limitada: sem orcamento, ou o filho nao decrementou')
   WHERE NEW.pai IS NOT NULL
     AND ( (SELECT delta FROM capacidade WHERE h = NEW.pai) < 1
        OR NEW.delta > (SELECT delta FROM capacidade WHERE h = NEW.pai) - 1 );

  -- compartilhamento controlado: atravessar a fronteira do tenant só com δ=0
  SELECT RAISE(ABORT, 'cross-tenant: atravessar a fronteira exige delta=0')
   WHERE NEW.pai IS NOT NULL AND NEW.delta > 0
     AND (SELECT p.banda FROM principal p WHERE p.h = NEW.detentor)
      <> (SELECT p.banda FROM principal p JOIN capacidade k ON k.detentor = p.h
           WHERE k.h = NEW.pai);

  -- não se delega a partir do que já foi negado
  SELECT RAISE(ABORT, 'deny domina: o pai esta revogado')
   WHERE NEW.pai IS NOT NULL AND EXISTS (SELECT 1 FROM deny d WHERE d.h = NEW.pai);
END;

-- a cadeia até a raiz — proveniência consultável sem decifrar nada
CREATE VIEW cadeia AS
WITH RECURSIVE c(h, ancestral, salto) AS (
    SELECT h, h, 0 FROM capacidade
  UNION ALL
    SELECT c.h, k.pai, c.salto + 1
      FROM c JOIN capacidade k ON k.h = c.ancestral
     WHERE k.pai IS NOT NULL
)
SELECT h, ancestral, salto FROM c;

-- 𝒞_δ: vigente é quem chega a uma raiz E não tem deny em si nem em ANCESTRAL nenhum.
-- O deny domina para baixo: revogar a raiz apaga a autoridade de toda a descendência
-- sem apagar linha nenhuma — que é o único jeito honesto num sistema que não tem o dado.
CREATE VIEW vigente AS
SELECT k.* FROM capacidade k
 WHERE EXISTS (SELECT 1 FROM cadeia c JOIN capacidade r ON r.h = c.ancestral
                WHERE c.h = k.h AND r.raiz = 1)
   AND NOT EXISTS (SELECT 1 FROM cadeia c JOIN deny d ON d.h = c.ancestral WHERE c.h = k.h);

-- Ψ = Collapse: o banco entrega o COFRE de quem está vigente — e nada mais. Quem decifra
-- é o usuário. Zero linhas é ⊥ (fail-closed): sem concessão, não devolve palpite.
CREATE VIEW colapso AS
SELECT h, detentor, delta, cofre FROM vigente;
