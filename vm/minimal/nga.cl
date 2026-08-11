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
;;;; machine for Common Lisp (SBCL).
;;;;
;;;; Copyright (c) 2008 - 2022, Charles Childers
;;;; Copyright (c) 2009 - 2010, Luke Parrish
;;;; Copyright (c) 2010,        Marc Simpson
;;;; Copyright (c) 2010,        Jay Skeer
;;;; Copyright (c) 2011,        Kenneth Keating

(defpackage :nga
  (:use :cl)
  (:export #:main))

(in-package :nga)

(defconstant +image-size+ 65536)
(defconstant +addresses+ 256)
(defconstant +stack-depth+ 256)
(defconstant +cell-min+ (+ (- (expt 2 31)) 1))
(defconstant +cell-max+ (- (expt 2 31) 2))

(deftype cell () '(signed-byte 32))

(defstruct nga-vm
  (sp 0 :type fixnum)
  (rp 0 :type fixnum)
  (ip 0 :type fixnum)
  (data (make-array +stack-depth+ :element-type 'cell :initial-element 0)
        :type (simple-array cell (*)))
  (address (make-array +addresses+ :element-type 'cell :initial-element 0)
           :type (simple-array cell (*)))
  (memory (make-array (1+ +image-size+) :element-type 'cell :initial-element 0)
          :type (simple-array cell (*))))

(defmacro tos (vm)
  `(aref (nga-vm-data ,vm) (nga-vm-sp ,vm)))

(defmacro nos (vm)
  `(aref (nga-vm-data ,vm) (1- (nga-vm-sp ,vm))))

(defmacro tors (vm)
  `(aref (nga-vm-address ,vm) (nga-vm-rp ,vm)))

(defun to-signed-32 (n)
  "Convert unsigned 32-bit value to signed 32-bit"
  (if (logbitp 31 n)
      (- n (ash 1 32))
      n))

(defun stack-push (vm value)
  (incf (nga-vm-sp vm))
  (setf (tos vm) value))

(defun stack-pop (vm)
  (prog1 (aref (nga-vm-data vm) (nga-vm-sp vm))
    (decf (nga-vm-sp vm))))

(defun load-image (vm image-file)
  (with-open-file (stream image-file
                          :direction :input
                          :element-type '(unsigned-byte 8)
                          :if-does-not-exist nil)
    (when stream
      (loop for i from 0 below +image-size+
            for byte1 = (read-byte stream nil nil)
            for byte2 = (read-byte stream nil nil)
            for byte3 = (read-byte stream nil nil)
            for byte4 = (read-byte stream nil nil)
            while (and byte1 byte2 byte3 byte4)
            do (let ((value (logior byte1
                                   (ash byte2 8)
                                   (ash byte3 16)
                                   (ash byte4 24))))
                 (setf (aref (nga-vm-memory vm) i)
                       (if (logbitp 31 value)
                           (- value (ash 1 32))
                           value)))))))

(defun prepare-vm (vm)
  (setf (nga-vm-ip vm) 0
        (nga-vm-sp vm) 0
        (nga-vm-rp vm) 0)
  (fill (nga-vm-memory vm) 0)
  (fill (nga-vm-data vm) 0)
  (fill (nga-vm-address vm) 0))

;;; Instructions

(defun inst-no (vm)
  (declare (ignore vm))
  nil)

(defun inst-li (vm)
  (incf (nga-vm-ip vm))
  (stack-push vm (aref (nga-vm-memory vm) (nga-vm-ip vm))))

(defun inst-du (vm)
  (stack-push vm (tos vm)))

(defun inst-dr (vm)
  (setf (aref (nga-vm-data vm) (nga-vm-sp vm)) 0)
  (decf (nga-vm-sp vm))
  (when (< (nga-vm-sp vm) 0)
    (setf (nga-vm-ip vm) +image-size+)))

(defun inst-sw (vm)
  (let ((a (tos vm)))
    (setf (tos vm) (nos vm)
          (nos vm) a)))

(defun inst-pu (vm)
  (incf (nga-vm-rp vm))
  (setf (tors vm) (tos vm))
  (inst-dr vm))

(defun inst-po (vm)
  (stack-push vm (tors vm))
  (decf (nga-vm-rp vm)))

(defun inst-ju (vm)
  (setf (nga-vm-ip vm) (1- (tos vm)))
  (inst-dr vm))

(defun inst-ca (vm)
  (incf (nga-vm-rp vm))
  (setf (tors vm) (nga-vm-ip vm))
  (setf (nga-vm-ip vm) (1- (tos vm)))
  (inst-dr vm))

(defun inst-cc (vm)
  (let ((a (tos vm)))
    (inst-dr vm)
    (let ((b (tos vm)))
      (inst-dr vm)
      (when (/= b 0)
        (incf (nga-vm-rp vm))
        (setf (tors vm) (nga-vm-ip vm))
        (setf (nga-vm-ip vm) (1- a))))))

(defun inst-re (vm)
  (setf (nga-vm-ip vm) (tors vm))
  (decf (nga-vm-rp vm)))

(defun inst-eq (vm)
  (setf (nos vm) (if (= (nos vm) (tos vm)) -1 0))
  (inst-dr vm))

(defun inst-ne (vm)
  (setf (nos vm) (if (/= (nos vm) (tos vm)) -1 0))
  (inst-dr vm))

(defun inst-lt (vm)
  (setf (nos vm) (if (< (nos vm) (tos vm)) -1 0))
  (inst-dr vm))

(defun inst-gt (vm)
  (setf (nos vm) (if (> (nos vm) (tos vm)) -1 0))
  (inst-dr vm))

(defun inst-fe (vm)
  (let ((addr (tos vm)))
    (setf (tos vm)
          (case addr
            (-1 (1- (nga-vm-sp vm)))
            (-2 (nga-vm-rp vm))
            (-3 +image-size+)
            (-4 +cell-min+)
            (-5 +cell-max+)
            (otherwise (aref (nga-vm-memory vm) addr))))))

(defun inst-st (vm)
  (let ((addr (tos vm)))
    (if (and (<= addr +image-size+) (>= addr 0))
        (progn
          (setf (aref (nga-vm-memory vm) addr) (nos vm))
          (inst-dr vm)
          (inst-dr vm))
        (setf (nga-vm-ip vm) +image-size+))))

(defun inst-ad (vm)
  (setf (nos vm) (to-signed-32 (logand #xFFFFFFFF (+ (nos vm) (tos vm)))))
  (inst-dr vm))

(defun inst-su (vm)
  (setf (nos vm) (to-signed-32 (logand #xFFFFFFFF (- (nos vm) (tos vm)))))
  (inst-dr vm))

(defun inst-mu (vm)
  (setf (nos vm) (to-signed-32 (logand #xFFFFFFFF (* (nos vm) (tos vm)))))
  (inst-dr vm))

(defun inst-di (vm)
  (let ((a (tos vm))
        (b (nos vm)))
    (setf (tos vm) (truncate b a)
          (nos vm) (mod b a))))

(defun inst-an (vm)
  (setf (nos vm) (logand (tos vm) (nos vm)))
  (inst-dr vm))

(defun inst-or (vm)
  (setf (nos vm) (logior (tos vm) (nos vm)))
  (inst-dr vm))

(defun inst-xo (vm)
  (setf (nos vm) (logxor (tos vm) (nos vm)))
  (inst-dr vm))

(defun inst-sh (vm)
  (let ((y (tos vm))
        (x (nos vm)))
    (setf (nos vm)
          (if (< y 0)
              (ash x (- y))
              (if (and (< x 0) (> y 0))
                  (logior (ash x (- y))
                          (lognot (ash (lognot 0) (- y))))
                  (ash x (- y)))))
    (inst-dr vm)))

(defun inst-zr (vm)
  (when (= (tos vm) 0)
    (inst-dr vm)
    (setf (nga-vm-ip vm) (tors vm))
    (decf (nga-vm-rp vm))))

(defun inst-ha (vm)
  (setf (nga-vm-ip vm) +image-size+))

(defun inst-ie (vm)
  (stack-push vm 2))

(defun inst-iq (vm)
  (cond
    ((= (tos vm) 0)
     (inst-dr vm)
     (stack-push vm 0)
     (stack-push vm 0))
    ((= (tos vm) 1)
     (inst-dr vm)
     (stack-push vm 1)
     (stack-push vm 1))
    (t
     (inst-dr vm))))

(defun inst-ii (vm)
  (cond
    ((= (tos vm) 0)
     (inst-dr vm)
     (let ((c (stack-pop vm)))
       (write-char (code-char (logand c #xFF)))
       (finish-output)))
    ((= (tos vm) 1)
     (let ((c (read-char *standard-input* nil nil)))
       (inst-dr vm)
       (if c
           (stack-push vm (char-code c))
           (sb-ext:exit))))
    (t
     (inst-dr vm))))

(defvar *instructions*
  (vector #'inst-no #'inst-li #'inst-du #'inst-dr #'inst-sw #'inst-pu #'inst-po
          #'inst-ju #'inst-ca #'inst-cc #'inst-re #'inst-eq #'inst-ne #'inst-lt
          #'inst-gt #'inst-fe #'inst-st #'inst-ad #'inst-su #'inst-mu #'inst-di
          #'inst-an #'inst-or #'inst-xo #'inst-sh #'inst-zr #'inst-ha #'inst-ie
          #'inst-iq #'inst-ii))

(defun process-opcode-bundle (vm opcode)
  (funcall (aref *instructions* (logand opcode #xFF)) vm)
  (funcall (aref *instructions* (logand (ash opcode -8) #xFF)) vm)
  (funcall (aref *instructions* (logand (ash opcode -16) #xFF)) vm)
  (funcall (aref *instructions* (logand (ash opcode -24) #xFF)) vm))

(defun execute (vm cell)
  (setf (nga-vm-rp vm) 1
        (nga-vm-ip vm) cell)
  (loop while (< (nga-vm-ip vm) +image-size+)
        do (let ((opcode (aref (nga-vm-memory vm) (nga-vm-ip vm))))
             (process-opcode-bundle vm opcode)
             (incf (nga-vm-ip vm))
             (when (= (nga-vm-rp vm) 0)
               (setf (nga-vm-ip vm) +image-size+)))))

(defun main ()
  (let ((vm (make-nga-vm)))
    (prepare-vm vm)
    (load-image vm "ngaImage")
    (handler-case
        (execute vm 0)
      (error () nil))))

(main)
