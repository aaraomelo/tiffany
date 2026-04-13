// Formats gateway tool results as YAML with summary for LLM consumption
// Less tokens than JSON, more readable, summary allows direct response

function toYaml(obj: any, indent = 0): string {
  const pad = '  '.repeat(indent);
  const lines: string[] = [];

  for (const [key, value] of Object.entries(obj)) {
    if (value === null || value === undefined) continue;

    if (Array.isArray(value)) {
      if (value.length === 0) {
        lines.push(`${pad}${key}: []`);
      } else if (typeof value[0] === 'object') {
        lines.push(`${pad}${key}:`);
        for (const item of value) {
          const itemLines = toYaml(item, indent + 2).split('\n').filter(Boolean);
          if (itemLines.length > 0) {
            lines.push(`${pad}  - ${itemLines[0].trim()}`);
            for (const l of itemLines.slice(1)) lines.push(`${pad}    ${l.trim()}`);
          }
        }
      } else {
        lines.push(`${pad}${key}:`);
        for (const item of value) lines.push(`${pad}  - ${item}`);
      }
    } else if (typeof value === 'object') {
      lines.push(`${pad}${key}:`);
      lines.push(toYaml(value, indent + 1));
    } else {
      lines.push(`${pad}${key}: ${value}`);
    }
  }

  return lines.join('\n');
}

export function formatToolResult(summary: string, data?: Record<string, any>): string {
  const parts = [`resumo: ${summary}`];
  if (data && Object.keys(data).length > 0) {
    parts.push('---');
    parts.push(toYaml(data));
  }
  return parts.join('\n');
}
