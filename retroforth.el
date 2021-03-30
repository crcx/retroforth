;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; retroforth-mode for emacs
;; version: 0.01
;; RETRO 12 syntax
;;
;; ----------------------------------------------------------
;; Copyright (c) 2021 Philippe Brochard <hocwp@free.fr>
;;
;; Permission to use, copy, modify, and/or distribute this software
;; for any purpose with or without fee is hereby granted, provided
;; that the above copyright notice and this permission notice appear
;; in all copies.
;;
;; THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
;; WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
;; WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
;; AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
;; CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
;; LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
;; NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
;; CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
;; ----------------------------------------------------------
;; ( http://en.wikipedia.org/wiki/ISC_license )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; usage : add this to your .emacs file:
;;
;; (add-to-list 'load-path <path/to/retroforth.el>)
;; (require 'retroforth)
;;
;; files in *.retro will be open in retroforth mode
;;
;; interactions:
;;
;;   C-M-x     retroforth-eval-block-region
;;   C-c C-k   forth-kill
;;   C-c C-f   forth-restart
;;   C-c :     forth-eval
;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; changelog
;;
;; 2021-03-30 v. 0.01 hocwp
;; retroforth-mode for emacs
;; limited to basic syntax highlighting
;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; implementation
;;


;----------------------------------------------------------
; forth-mode require for basic usage already defined in this mode:
;
(require 'forth-mode)



;----------------------------------------------------------
; retroforth groups definitions
;
(defgroup retroforth nil
  "Major mode for editing and running RETRO Forth."
  :prefix "retrofoth-"
  :link '(url-link "http://www.retroforth.org/"))

(defgroup retroforth-lock-faces nil
  "Major mode for editing and running RETRO Forth."
  :prefix "retrofoth-"
  :group 'retroforth
  :link '(url-link "http://www.retroforth.org/"))


;----------------------------------------------------------
; tables
; start with empty syntax and abbrev tables
;
(defvar retroforth-mode-syntax-table (make-syntax-table) "")
(defvar retroforth-mode-abbrev-table (make-abbrev-table) "")

(defvar retroforth-block-regex "~~~\\|```")


;----------------------------------------------------------
; face definitions for syntax highlighting
; ( instead of editing the colors here, use M-x customize-face )
;
;;; Markdown faces
(defface retroforth-markdown-title-face
  '((t (:foreground "blue")))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-markdown-italic-face
  '((t (:inherit italic)))
  "Face for italic text."
  :group 'retroforth-lock-faces)

(defface retroforth-markdown-bold-face
  '((t (:inherit bold)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-markdown-inline-code-face
	'((t (:foreground "dark cyan")))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

;;; Code faces
(defface retroforth-block-highlight
  '((t (:background "seashell" :extend t)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-define-face
  '((t (:foreground "red" :inherit retroforth-block-highlight)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-compile-face
  '((t (:foreground "green4" :inherit retroforth-block-highlight)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-execute-face
  '((t (:foreground "yellow4" :inherit retroforth-block-highlight)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-number-face
  '((t (:foreground "dark blue" :inherit retroforth-block-highlight)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-string-face
  '((t (:foreground "VioletRed4" :inherit retroforth-block-highlight)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-comment-face
  '((t (:foreground "grey40" :inherit retroforth-block-highlight)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-bracket-face
  '((t (:foreground "darkorange3" :inherit retroforth-block-highlight)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-code-face
  '((t (:foreground "grey40" :inherit retroforth-block-highlight)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-var-face
  '((t (:foreground "dark magenta" :inherit retroforth-block-highlight)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-keyword-face
  '((t (:foreground "dark blue" :inherit retroforth-block-highlight)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defface retroforth-highlight-face
  '((t (:background "azure2" :extend t)))
  "Face for retroforth mode."
  :group 'retroforth-lock-faces)

(defun defprefixhighlight (prefix face)
  (cons (format "\\(^\\|[ \n\t]\\)%s[^ \n\t]+" prefix) `(0 ',face t)))

(setq retroforth-highlights
	  (let* ((keywords '("bye" "TIB" "abort" "include" "FREE" "dump-stack" "reset"
						 "tab" "sp" "nl" "unhook" "set-hook" "hook" "indexed-times"
						 "K" "J" "I" "does" "curry" "reorder" "STRINGS" "TempStringMax"
						 "TempStrings" "---reveal---" "ScopeList" "copy" "allot" "mod"
						 "rot" "-if;" "if;" "gteq?" "lteq?" "not" "case" "FALSE" "TRUE"
						 "times" "until" "while" "tri@" "tri*" "tri" "bi@" "bi*" "bi" "sip"
						 "dip" "dup-pair" "?dup" "drop-pair" "nip" "over" "tuck" "const"
						 "var" "var-n" "here" "compiling?" "primitive" "data" "immediate"
						 "reclass" "depth" "EOM" "Version" "interpret" "again" "repeat" "Dictionary"
						 "Heap" "Compiler" "-if" "if" "choose" "store-next" "fetch-next" "pop"
						 "push" "shift" "xor" "or" "and" "/mod" "store" "fetch"
						 "gt?" "lt?" "-eq?" "eq?" "call" "swap" "drop" "dup"))
			 (keywords-regexp (regexp-opt keywords 'words)))
		`(("\\(~~~\\|```\\)\\(.\\|\n\\)*?\\(~~~\\|```\\)" . (0 'retroforth-block-highlight t))
		  ("[]{}\\[]" . (0 'retroforth-bracket-face t))
		  ("^#.*$" . (0 'retroforth-markdown-title-face t))
		  (,keywords-regexp . (0 'retroforth-keyword-face t))
		  (" \\*.*\\*" . (0 'retroforth-markdown-bold-face t))
		  (" _.*_" . (0 'retroforth-markdown-italic-face t))
		  (" `.*`" . (0 'retroforth-markdown-inline-code-face t))
		  ,(defprefixhighlight "\\w+:" 'retroforth-compile-face)
		  ,(defprefixhighlight "#[0-9]+" 'retroforth-number-face)
		  ,(defprefixhighlight "\\." 'retroforth-number-face)
		  ("\\$." (0 'retroforth-number-face t))
		  ,(defprefixhighlight "'" 'retroforth-string-face)
		  ,(defprefixhighlight ":" 'retroforth-define-face)
		  (" ;" . (0 'retroforth-define-face t))
		  ,(defprefixhighlight "(" 'retroforth-comment-face)
		  ,(defprefixhighlight "&" 'retroforth-var-face)
		  ,(defprefixhighlight "@" 'retroforth-var-face)
		  ,(defprefixhighlight "!" 'retroforth-var-face))))


;----------------------------------------------------------
; forth mode interactions
;
(defun retroforth-interaction-send (&rest strings)
  (forth-scrub (or (apply #'forth-interaction-send-raw-result strings)
				   "OK: No output")))

(defun retroforth-eval (string)
  (interactive "sForth expression: ")
  (message "%s" (retroforth-interaction-send string)))

(defun retroforth-eval-region (start end)
  (interactive "r")
  (retroforth-eval (buffer-substring start end)))

(defun retroforth-eval-block-region ()
  (interactive)
  (save-excursion
   (re-search-backward retroforth-block-regex) (next-line)
   (beginning-of-line)
   (let ((beg (point)))
	 (re-search-forward retroforth-block-regex)
	 (beginning-of-line)
	 (pulse-momentary-highlight-region beg (point) 'retroforth-highlight-face)
	 (retroforth-eval-region beg (point)))))

(defvar retroforth-mode-map
  (let ((map (make-sparse-keymap)))
	(define-key map (kbd "C-M-x")     'retroforth-eval-block-region)
	(define-key map (kbd "C-c C-k")   'forth-kill)
	(define-key map (kbd "C-c C-f")   'forth-restart)
	(define-key map (kbd "C-c :")     'forth-eval)
	map))


;----------------------------------------------------------
; blocks highlight function
;
(defun retroforth-font-lock-extend-region ()
  "Extend the search region to include an entire block of text."
  ;; Avoid compiler warnings about these global variables from font-lock.el.
  ;; See the documentation for variable `font-lock-extend-region-functions'.
  (eval-when-compile (defvar font-lock-beg) (defvar font-lock-end))
  (save-excursion
	(goto-char font-lock-beg)
	(let ((found (or (re-search-backward "\n\n" nil t) (point-min))))
	  (goto-char font-lock-end)
	  (when (re-search-forward "\n\n" nil t)
		(beginning-of-line)
		(setq font-lock-end (point)))
	  (setq font-lock-beg found))))



;----------------------------------------------------------
; retroforth mode
;
(define-derived-mode retroforth-mode text-mode "retroforth"
  "major mode for editing RETRO Forth language code."
  (setq font-lock-defaults '(retroforth-highlights))
  (set (make-local-variable 'font-lock-multiline) t)
  (add-hook 'font-lock-extend-region-functions
			'retroforth-font-lock-extend-region)
  (use-local-map retroforth-mode-map))

(setq auto-mode-alist (cons '("\\.retro\\'" . retroforth-mode)
							auto-mode-alist))

(provide 'retroforth)

;; end.
