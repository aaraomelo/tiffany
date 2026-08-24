(module
  ;; ============================================================
  ;; neuronio8.wat
  ;;
  ;; Byte -> base ortonormal de 8
  ;;
  ;;   bit 0 -> fase 0 -> Lei 0
  ;;   bit 1 -> fase 1 -> Lei 1
  ;;   bit 2 -> fase 2 -> Lei 2
  ;;   bit 3 -> fase 3 -> Lei 3
  ;;   bit 4 -> fase 4 -> Lei 4
  ;;   bit 5 -> fase 5 -> Lei 5
  ;;   bit 6 -> fase 6 -> Lei 6
  ;;   bit 7 -> fase 7 -> Lei 7
  ;;
  ;; Resultado:
  ;;
  ;;   [total,f0,f1,f2,f3,f4,f5,f6,f7]
  ;;
  ;; O host fornece:
  ;;   ptr = início do buffer
  ;;   len = número de bytes
  ;;
  ;; Retorno:
  ;;   ponteiro para 9 u64:
  ;;
  ;;     result[0] = total
  ;;     result[1] = f0
  ;;     ...
  ;;     result[8] = f7
  ;; ============================================================

  (memory (export "memory") 1)

  ;; Área de resultado:
  ;; 9 x 8 = 72 bytes
  (global $result (mut i32) (i32.const 0))

  ;; ------------------------------------------------------------
  ;; neuronio8(ptr, len) -> result_ptr
  ;; ------------------------------------------------------------

  (func (export "neuronio8")
    (param $ptr i32)
    (param $len i32)
    (result i32)

    ;; ----------------------------------------------------------
    ;; Contadores locais.
    ;; Cada fase é independente.
    ;; ----------------------------------------------------------

    (local $f0 i64)
    (local $f1 i64)
    (local $f2 i64)
    (local $f3 i64)
    (local $f4 i64)
    (local $f5 i64)
    (local $f6 i64)
    (local $f7 i64)

    (local $i i32)
    (local $b i32)

    (local.set $i (i32.const 0))

    ;; ----------------------------------------------------------
    ;; streaming sobre o buffer
    ;; ----------------------------------------------------------

    (block $done
      (loop $read

        (br_if $done
          (i32.ge_u
            (local.get $i)
            (local.get $len)))

        ;; byte atual
        (local.set $b
          (i32.load8_u
            (i32.add
              (local.get $ptr)
              (local.get $i))))

        ;; ------------------------------------------------------
        ;; Cada bit é uma coordenada.
        ;; ------------------------------------------------------

        (local.set $f0
          (i64.add
            (local.get $f0)
            (i64.extend_i32_u
              (i32.and
                (i32.shr_u (local.get $b) (i32.const 0))
                (i32.const 1)))))

        (local.set $f1
          (i64.add
            (local.get $f1)
            (i64.extend_i32_u
              (i32.and
                (i32.shr_u (local.get $b) (i32.const 1))
                (i32.const 1)))))

        (local.set $f2
          (i64.add
            (local.get $f2)
            (i64.extend_i32_u
              (i32.and
                (i32.shr_u (local.get $b) (i32.const 2))
                (i32.const 1)))))

        (local.set $f3
          (i64.add
            (local.get $f3)
            (i64.extend_i32_u
              (i32.and
                (i32.shr_u (local.get $b) (i32.const 3))
                (i32.const 1)))))

        (local.set $f4
          (i64.add
            (local.get $f4)
            (i64.extend_i32_u
              (i32.and
                (i32.shr_u (local.get $b) (i32.const 4))
                (i32.const 1)))))

        (local.set $f5
          (i64.add
            (local.get $f5)
            (i64.extend_i32_u
              (i32.and
                (i32.shr_u (local.get $b) (i32.const 5))
                (i32.const 1)))))

        (local.set $f6
          (i64.add
            (local.get $f6)
            (i64.extend_i32_u
              (i32.and
                (i32.shr_u (local.get $b) (i32.const 6))
                (i32.const 1)))))

        (local.set $f7
          (i64.add
            (local.get $f7)
            (i64.extend_i32_u
              (i32.and
                (i32.shr_u (local.get $b) (i32.const 7))
                (i32.const 1)))))

        (local.set $i
          (i32.add (local.get $i) (i32.const 1)))

        (br $read)
      )
    )

    ;; ----------------------------------------------------------
    ;; total = f0 + ... + f7
    ;; ----------------------------------------------------------

    ;; result[0]
    (i64.store
      (global.get $result)
      (i64.add
        (i64.add
          (i64.add
            (i64.add
              (local.get $f0)
              (local.get $f1))
            (i64.add
              (local.get $f2)
              (local.get $f3)))
          (i64.add
            (local.get $f4)
            (local.get $f5)))
        (i64.add
          (local.get $f6)
          (local.get $f7))))

    ;; result[1..8]
    (i64.store offset=8
      (global.get $result) (local.get $f0))

    (i64.store offset=16
      (global.get $result) (local.get $f1))

    (i64.store offset=24
      (global.get $result) (local.get $f2))

    (i64.store offset=32
      (global.get $result) (local.get $f3))

    (i64.store offset=40
      (global.get $result) (local.get $f4))

    (i64.store offset=48
      (global.get $result) (local.get $f5))

    (i64.store offset=56
      (global.get $result) (local.get $f6))

    (i64.store offset=64
      (global.get $result) (local.get $f7))

    (global.get $result)
  )
)