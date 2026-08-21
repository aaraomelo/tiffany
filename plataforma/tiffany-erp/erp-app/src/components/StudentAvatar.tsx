import { useEffect, useState } from 'react'
import { Avatar } from '@mui/material'
import { fetchStudentPhotoUrl } from '../api'

interface Props {
  studentId: string
  name: string
  /** Muda quando a foto é atualizada → dispara recarga e quebra cache. */
  photoUpdatedAt?: string | null
  size?: number
}

function initials(name: string): string {
  const parts = name.trim().split(/\s+/).filter(Boolean)
  if (parts.length === 0) return '?'
  if (parts.length === 1) return parts[0].slice(0, 2).toUpperCase()
  return (parts[0][0] + parts[parts.length - 1][0]).toUpperCase()
}

export function StudentAvatar({ studentId, name, photoUpdatedAt, size = 40 }: Props) {
  const [url, setUrl] = useState<string | null>(null)

  useEffect(() => {
    let active = true
    let objectUrl: string | null = null

    if (!photoUpdatedAt) {
      setUrl(null)
      return
    }

    void fetchStudentPhotoUrl(studentId, photoUpdatedAt).then((u) => {
      if (!active) {
        if (u) URL.revokeObjectURL(u)
        return
      }
      objectUrl = u
      setUrl(u)
    })

    return () => {
      active = false
      if (objectUrl) URL.revokeObjectURL(objectUrl)
    }
  }, [studentId, photoUpdatedAt])

  return (
    <Avatar src={url ?? undefined} sx={{ width: size, height: size, fontSize: size * 0.4 }}>
      {!url && initials(name)}
    </Avatar>
  )
}
