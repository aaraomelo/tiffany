# O Patria como runner — instalação

**Porquê.** A 06/08 o GitHub Actions esteve em `major_outage` e cinco corridas
seguidas morreram sem começar: *«the job was not acquired by Runner of type
hosted»*. Não havia nada a corrigir deste lado. Um runner nosso não depende da
frota deles — e, como o servidor **é** o destino da publicação, o deploy fica
mais simples: sem `ssh`, sem `scp`, sem chave privada guardada nos segredos.

---

## Antes de instalar: a questão de segurança, que é real

O repositório é **público**. O GitHub desaconselha runners próprios em repos
públicos, e a razão é concreta: quem faça um fork pode abrir um *pull request*
cujo workflow **corre código na sua máquina**.

**O que fecha isso aqui, hoje:**

- o `publica-local.yml` dispara **só** por `workflow_dispatch` — ninguém o
  aciona sem permissão de escrita no repositório;
- **nenhum** workflow deste repositório tem gatilho `pull_request`;
- o runner corre com um utilizador **sem privilégios** (a seguir).

> **Se algum dia acrescentar um gatilho `pull_request` a qualquer workflow deste
> repositório, esta análise deixa de valer e o runner tem de sair.** Fica escrito
> aqui porque é o tipo de coisa que se esquece seis meses depois.

---

## 1. Um utilizador só para isto

Não corra o runner como `root`. Se alguém conseguir executar código nele, a
diferença entre `root` e um utilizador sem privilégios é a diferença entre
perder o servidor e perder um diretório.

```bash
adduser --disabled-password --gecos "" runner
usermod -aG www-data runner          # para poder escrever na publicação
mkdir -p /var/www/goldenkingdom
chown -R runner:www-data /var/www/goldenkingdom
```

## 2. As ferramentas

O runner local não é descartável: instala-se uma vez.

```bash
apt-get update
apt-get install -y --no-install-recommends \
  texlive-latex-recommended texlive-latex-extra texlive-fonts-recommended \
  texlive-lang-portuguese poppler-utils rsync curl tar git
curl -fsSL https://deb.nodesource.com/setup_22.x | bash - && apt-get install -y nodejs
```

## 3. Registar o runner

O *token* obtém-se em
`Settings → Actions → Runners → New self-hosted runner` (é válido uma hora).

```bash
su - runner
mkdir actions-runner && cd actions-runner
curl -o r.tar.gz -L https://github.com/actions/runner/releases/download/v2.319.1/actions-runner-linux-x64-2.319.1.tar.gz
tar xzf r.tar.gz

./config.sh --url https://github.com/aaraomelo/tiffany \
            --token COLE_O_TOKEN_AQUI \
            --name patria --labels patria --unattended
```

O rótulo **`patria`** não é decorativo: o workflow pede
`runs-on: [self-hosted, patria]`, e sem ele o trabalho não encontra a máquina.

## 4. Pôr como serviço

```bash
exit                                  # voltar a root
cd /home/runner/actions-runner
./svc.sh install runner
./svc.sh start
./svc.sh status
```

## 5. Provar que funciona

```bash
gh workflow run publica-local.yml --ref master
gh run watch
```

O workflow verifica sozinho no fim: os cinco PDFs têm de responder `200` **e**
com `content-type: application/pdf`. O `200` sozinho não chega — o *SPA
fallback* do nginx devolve `200` com `text/html` para um PDF que não existe, e
já enganou uma publicação inteira.

---

## O que fica, e o que não muda

O `publica.yml` **continua a existir e a ser o principal**. Este é a saída para
quando a frota do GitHub cai. Trocar o `runs-on` do workflow principal deixaria
a publicação presa no dia em que o runner local parasse — dois caminhos que não
dependem um do outro valem mais do que um caminho novo a substituir o antigo.
