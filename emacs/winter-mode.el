;;(load "/usr/share/emacs/24.5/lisp/emacs-lisp/regexp-opt.el")

(defvar winter-mode-hook nil)

(defvar winter-mode-map
  (let ((map (make-keymap)))
    map)
  "Keymap for Winter major mode")

(add-to-list 'auto-mode-alist '("\\.w\\'" . winter-mode))

(defconst winter-keywords
  '("none" "true" "false" "return" "if"
	"else" "func" "loop" "break" "continue"
	"or" "and" "as" "int" "float" "bool"
	"string" "list" "while" "record"))

(defun get-winter-keywords ()
  (regexp-opt winter-keywords 'symbols))

(defconst winter-font-lock-keywords-1
  (list `(,(get-winter-keywords) . font-lock-keyword-face))
  "Minimal highlighting for Winter mode")

(defvar winter-font-lock-keywords
  winter-font-lock-keywords-1
  "Default highlighting expressions for Winter mode")

(defun winter-indent-line ()
  "Indent current line as Winter source"
  (interactive)
  (beginning-of-line)
  (if (bobp) ;; If at start of buffer
	  (indent-line-to 0)
	(let ((not-indented t) cur-indent)
	  (if (looking-at "^[ \t]*}") ;; If at end of scope
		  (progn
			(save-excursion
			  (forward-line -1)
			  (setq cur-indent (- (current-indentation) default-tab-width)))
			  (if (< cur-indent 0)
				  (setq cur-indent 0)))
		(save-excursion
		  (while not-indented
			(forward-line -1)
			(if (looking-at "^[ \t]*\\(}\\|)\\|\\]\\)")
				(progn
				  (setq cur-indent (current-indentation))
				  (setq not-indented nil))
			  (if (looking-at "^[ \\t]*.*\\({\\|(\\|\\[\\)")
				  (progn
					(setq cur-indent (+ (current-indentation) default-tab-width))
					(setq not-indented nil))
				(if (bobp)
					(setq not-indented nil)))))))
	  (if cur-indent
		  (indent-line-to cur-indent)
		(indent-line-to 0)))))

(defun winter-mode ()
  "Major mode for editing Winter source files"
  (interactive)
  (kill-all-local-variables)
  ;;(set-syntax-table winter-mode-syntax-table)
  (use-local-map winter-mode-map)
  (set (make-local-variable 'font-lock-defaults) '(winter-font-lock-keywords))
  (set (make-local-variable 'indent-line-function) 'winter-indent-line)
  (setq major-mode 'winter-mode)
  (setq mode-name "Winter")
  (run-hooks 'winter-mode-hook))

(provide 'winter-mode)
