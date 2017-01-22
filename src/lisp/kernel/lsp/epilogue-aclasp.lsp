#+(or clasp-min aclasp)
(eval-when (:load-toplevel)
  (unwind-protect
       (process-command-line-load-eval-sequence))
  (core::select-package :core)
  (let ((core:*use-interpreter-for-eval* nil))
    (when (member :interactive *features*) (core::low-level-repl))))
