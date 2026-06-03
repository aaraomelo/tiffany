// Redimensiona uma imagem no navegador (canvas) antes do upload, pra não
// trafegar/armazenar arquivos grandes. Foto de aluno é só pra desambiguar visualmente,
// então um thumbnail de ~400px em JPEG é mais que suficiente (~30–80KB).

export interface ResizedImage {
  /** Data URL "data:image/jpeg;base64,..." pronto pra enviar. */
  dataUrl: string
  mimeType: string
}

export async function resizeImageFile(
  file: File,
  maxSize = 400,
  quality = 0.82,
): Promise<ResizedImage> {
  const dataUrl = await readFileAsDataUrl(file)
  const img = await loadImage(dataUrl)

  let { width, height } = img
  if (width > maxSize || height > maxSize) {
    if (width >= height) {
      height = Math.round((height * maxSize) / width)
      width = maxSize
    } else {
      width = Math.round((width * maxSize) / height)
      height = maxSize
    }
  }

  const canvas = document.createElement('canvas')
  canvas.width = width
  canvas.height = height
  const ctx = canvas.getContext('2d')
  if (!ctx) throw new Error('Canvas não suportado')
  ctx.drawImage(img, 0, 0, width, height)

  // PNG com transparência perde o fundo ao virar JPEG; pintamos branco antes.
  const mimeType = 'image/jpeg'
  return { dataUrl: canvas.toDataURL(mimeType, quality), mimeType }
}

function readFileAsDataUrl(file: File): Promise<string> {
  return new Promise((resolve, reject) => {
    const reader = new FileReader()
    reader.onload = () => resolve(reader.result as string)
    reader.onerror = () => reject(new Error('Falha ao ler o arquivo'))
    reader.readAsDataURL(file)
  })
}

function loadImage(src: string): Promise<HTMLImageElement> {
  return new Promise((resolve, reject) => {
    const img = new Image()
    img.onload = () => resolve(img)
    img.onerror = () => reject(new Error('Imagem inválida'))
    img.src = src
  })
}
