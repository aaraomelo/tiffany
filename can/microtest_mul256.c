/* Microteste: multiplicacao por 256 via composicao de ADD
 * 
 * A Word16 (a, b) representa a + 256*b
 * Multiplicar por 256: (a, b) -> (0, a)
 * 
 * Estrategia:
 * 1. TROCA troca os componentes: (a, b) -> (b, a)
 * 2. SUB a si mesmo: (b, a) -> (b-a, a) -- nao funciona
 * 
 * Alternativa: usar STORE_IND para gravar bytes diretamente
 * 1. LOAD byte_x -> R
 * 2. STORE_IND slot_y  -- grava R no slot_y
 * 
 * Vamos testar se podemos construir BE length 2:
 * payload: [b0, b1]
 * BE: b0 * 256 + b1 = (b1, b0) como Word16
 * Ou seja: raw_out[0] = b1, raw_out[1] = b0
 * 
 * Para LE: raw_out[0] = b0, raw_out[1] = b1
 * 
 * A extração BE precisa reordenar os bytes.
 * Sem SHL, podemos apenas gravar os bytes na ordem inversa.
 * 
 * Mas o valor precisa ser correto como uint16.
 * BE: b0 * 256 + b1
 * LE: b1 * 256 + b0
 * 
 * Para representar como Word16 (low, high):
 * BE: (b1, b0) -- low=b1, high=b0
 * LE: (b0, b1) -- low=b0, high=b1
 * 
 * Entao para BE length 2:
 * raw_out[0] = b1  -- low byte
 * raw_out[1] = b0  -- high byte
 * 
 * Para LE length 2:
 * raw_out[0] = b0  -- low byte
 * raw_out[1] = b1  -- high byte
 * 
 * A extração BE precisa inverter a ordem dos bytes!
 * 
 * Vamos testar com o executor REAL.