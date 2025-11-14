;;;; RETRO is a clean, elegant, and pragmatic dialect of Forth.
;;;; It provides a simple alternative for those willing to make a
;;;; break from legacy systems.
;;;;
;;;; The language draws influences from many sources including
;;;; traditional Forth systems, cmForth, colorForth, Factor, and
;;;; Parable. It was designed to be easy to grasp and adapt to
;;;; specific uses.
;;;;
;;;; The basic language is very portable and runs on a tiny
;;;; virtual machine.
;;;;
;;;; This file contains a minimal implementation of the virtual
;;;; machine for Chicken Scheme.
;;;;
;;;; Copyright (c) 2008 - 2022, Charles Childers
;;;; Copyright (c) 2009 - 2010, Luke Parrish
;;;; Copyright (c) 2010,        Marc Simpson
;;;; Copyright (c) 2010,        Jay Skeer
;;;; Copyright (c) 2011,        Kenneth Keating

(import (chicken io)
        (chicken port)
        (chicken file)
        (chicken bitwise))

(define +image-size+ 65536)
(define +addresses+ 256)
(define +stack-depth+ 256)
(define +cell-min+ (+ (- (expt 2 31)) 1))
(define +cell-max+ (- (expt 2 31) 2))

(define-record nga-vm
  sp rp ip
  data address memory)

(define (make-new-vm)
  (make-nga-vm
    0 0 0
    (make-vector +stack-depth+ 0)
    (make-vector +addresses+ 0)
    (make-vector (+ +image-size+ 1) 0)))

(define (tos vm)
  (vector-ref (nga-vm-data vm) (nga-vm-sp vm)))

(define (nos vm)
  (vector-ref (nga-vm-data vm) (- (nga-vm-sp vm) 1)))

(define (tors vm)
  (vector-ref (nga-vm-address vm) (nga-vm-rp vm)))

(define (set-tos! vm value)
  (vector-set! (nga-vm-data vm) (nga-vm-sp vm) value))

(define (set-nos! vm value)
  (vector-set! (nga-vm-data vm) (- (nga-vm-sp vm) 1) value))

(define (set-tors! vm value)
  (vector-set! (nga-vm-address vm) (nga-vm-rp vm) value))

(define (to-signed-32 n)
  (let ((n32 (bitwise-and n #xFFFFFFFF)))
    (if (> n32 #x7FFFFFFF)
        (- n32 #x100000000)
        n32)))

(define (stack-push! vm value)
  (nga-vm-sp-set! vm (+ (nga-vm-sp vm) 1))
  (set-tos! vm value))

(define (stack-pop! vm)
  (let ((value (tos vm)))
    (nga-vm-sp-set! vm (- (nga-vm-sp vm) 1))
    value))

(define (load-image! vm image-file)
  (when (file-exists? image-file)
    (with-input-from-file image-file
      (lambda ()
        (let loop ((i 0))
          (let ((b1 (read-byte))
                (b2 (read-byte))
                (b3 (read-byte))
                (b4 (read-byte)))
            (when (and (not (eof-object? b1))
                       (not (eof-object? b2))
                       (not (eof-object? b3))
                       (not (eof-object? b4)))
              (let ((value (bitwise-ior b1
                                       (arithmetic-shift b2 8)
                                       (arithmetic-shift b3 16)
                                       (arithmetic-shift b4 24))))
                (vector-set! (nga-vm-memory vm) i (to-signed-32 value))
                (loop (+ i 1))))))))))

(define (prepare-vm! vm)
  (nga-vm-ip-set! vm 0)
  (nga-vm-sp-set! vm 0)
  (nga-vm-rp-set! vm 0)
  (vector-fill! (nga-vm-memory vm) 0)
  (vector-fill! (nga-vm-data vm) 0)
  (vector-fill! (nga-vm-address vm) 0))

;;; Instructions

(define (inst-no vm) #f)

(define (inst-li vm)
  (nga-vm-ip-set! vm (+ (nga-vm-ip vm) 1))
  (stack-push! vm (vector-ref (nga-vm-memory vm) (nga-vm-ip vm))))

(define (inst-du vm)
  (stack-push! vm (tos vm)))

(define (inst-dr vm)
  (vector-set! (nga-vm-data vm) (nga-vm-sp vm) 0)
  (nga-vm-sp-set! vm (- (nga-vm-sp vm) 1))
  (when (< (nga-vm-sp vm) 0)
    (nga-vm-ip-set! vm +image-size+)))

(define (inst-sw vm)
  (let ((a (tos vm)))
    (set-tos! vm (nos vm))
    (set-nos! vm a)))

(define (inst-pu vm)
  (nga-vm-rp-set! vm (+ (nga-vm-rp vm) 1))
  (set-tors! vm (tos vm))
  (inst-dr vm))

(define (inst-po vm)
  (stack-push! vm (tors vm))
  (nga-vm-rp-set! vm (- (nga-vm-rp vm) 1)))

(define (inst-ju vm)
  (nga-vm-ip-set! vm (- (tos vm) 1))
  (inst-dr vm))

(define (inst-ca vm)
  (nga-vm-rp-set! vm (+ (nga-vm-rp vm) 1))
  (set-tors! vm (nga-vm-ip vm))
  (nga-vm-ip-set! vm (- (tos vm) 1))
  (inst-dr vm))

(define (inst-cc vm)
  (let ((a (tos vm)))
    (inst-dr vm)
    (let ((b (tos vm)))
      (inst-dr vm)
      (when (not (= b 0))
        (nga-vm-rp-set! vm (+ (nga-vm-rp vm) 1))
        (set-tors! vm (nga-vm-ip vm))
        (nga-vm-ip-set! vm (- a 1))))))

(define (inst-re vm)
  (nga-vm-ip-set! vm (tors vm))
  (nga-vm-rp-set! vm (- (nga-vm-rp vm) 1)))

(define (inst-eq vm)
  (set-nos! vm (if (= (nos vm) (tos vm)) -1 0))
  (inst-dr vm))

(define (inst-ne vm)
  (set-nos! vm (if (not (= (nos vm) (tos vm))) -1 0))
  (inst-dr vm))

(define (inst-lt vm)
  (set-nos! vm (if (< (nos vm) (tos vm)) -1 0))
  (inst-dr vm))

(define (inst-gt vm)
  (set-nos! vm (if (> (nos vm) (tos vm)) -1 0))
  (inst-dr vm))

(define (inst-fe vm)
  (let ((addr (tos vm)))
    (set-tos! vm
      (cond
        ((= addr -1) (- (nga-vm-sp vm) 1))
        ((= addr -2) (nga-vm-rp vm))
        ((= addr -3) +image-size+)
        ((= addr -4) +cell-min+)
        ((= addr -5) +cell-max+)
        (else (vector-ref (nga-vm-memory vm) addr))))))

(define (inst-st vm)
  (let ((addr (tos vm)))
    (if (and (<= addr +image-size+) (>= addr 0))
        (begin
          (vector-set! (nga-vm-memory vm) addr (nos vm))
          (inst-dr vm)
          (inst-dr vm))
        (nga-vm-ip-set! vm +image-size+))))

(define (inst-ad vm)
  (set-nos! vm (to-signed-32 (+ (nos vm) (tos vm))))
  (inst-dr vm))

(define (inst-su vm)
  (set-nos! vm (to-signed-32 (- (nos vm) (tos vm))))
  (inst-dr vm))

(define (inst-mu vm)
  (set-nos! vm (to-signed-32 (* (nos vm) (tos vm))))
  (inst-dr vm))

(define (inst-di vm)
  (let ((a (tos vm))
        (b (nos vm)))
    (set-tos! vm (quotient b a))
    (set-nos! vm (remainder b a))))

(define (inst-an vm)
  (set-nos! vm (bitwise-and (tos vm) (nos vm)))
  (inst-dr vm))

(define (inst-or vm)
  (set-nos! vm (bitwise-ior (tos vm) (nos vm)))
  (inst-dr vm))

(define (inst-xo vm)
  (set-nos! vm (bitwise-xor (tos vm) (nos vm)))
  (inst-dr vm))

(define (inst-sh vm)
  (let ((y (tos vm))
        (x (nos vm)))
    (set-nos! vm
      (if (< y 0)
          (arithmetic-shift x (- y))
          (arithmetic-shift x (- y))))
    (inst-dr vm)))

(define (inst-zr vm)
  (when (= (tos vm) 0)
    (inst-dr vm)
    (nga-vm-ip-set! vm (tors vm))
    (nga-vm-rp-set! vm (- (nga-vm-rp vm) 1))))

(define (inst-ha vm)
  (nga-vm-ip-set! vm +image-size+))

(define (inst-ie vm)
  (stack-push! vm 2))

(define (inst-iq vm)
  (cond
    ((= (tos vm) 0)
     (inst-dr vm)
     (stack-push! vm 0)
     (stack-push! vm 0))
    ((= (tos vm) 1)
     (inst-dr vm)
     (stack-push! vm 1)
     (stack-push! vm 1))
    (else
     (inst-dr vm))))

(define (inst-ii vm)
  (cond
    ((= (tos vm) 0)
     (inst-dr vm)
     (let ((c (stack-pop! vm)))
       (write-char (integer->char (bitwise-and c #xFF)))
       (flush-output)))
    ((= (tos vm) 1)
     (let ((c (read-char)))
       (inst-dr vm)
       (if (eof-object? c)
           (exit 0)
           (stack-push! vm (char->integer c)))))
    (else
     (inst-dr vm))))

(define *instructions*
  (vector inst-no inst-li inst-du inst-dr inst-sw inst-pu inst-po
          inst-ju inst-ca inst-cc inst-re inst-eq inst-ne inst-lt
          inst-gt inst-fe inst-st inst-ad inst-su inst-mu inst-di
          inst-an inst-or inst-xo inst-sh inst-zr inst-ha inst-ie
          inst-iq inst-ii))

(define (process-opcode-bundle vm opcode)
  ((vector-ref *instructions* (bitwise-and opcode #xFF)) vm)
  ((vector-ref *instructions* (bitwise-and (arithmetic-shift opcode -8) #xFF)) vm)
  ((vector-ref *instructions* (bitwise-and (arithmetic-shift opcode -16) #xFF)) vm)
  ((vector-ref *instructions* (bitwise-and (arithmetic-shift opcode -24) #xFF)) vm))

(define (execute! vm cell)
  (nga-vm-rp-set! vm 1)
  (nga-vm-ip-set! vm cell)
  (let loop ()
    (when (< (nga-vm-ip vm) +image-size+)
      (let ((opcode (vector-ref (nga-vm-memory vm) (nga-vm-ip vm))))
        (process-opcode-bundle vm opcode)
        (nga-vm-ip-set! vm (+ (nga-vm-ip vm) 1))
        (if (= (nga-vm-rp vm) 0)
            (nga-vm-ip-set! vm +image-size+)
            (loop))))))

(define (main)
  (let ((vm (make-new-vm)))
    (prepare-vm! vm)
    (load-image! vm "ngaImage")
    (execute! vm 0)))

(main)
