#! /usr/bin/env sbcl --noinform

(defun read-dtrace-header (stream &optional eofp eof)
  (list (read-line stream eofp eof)
        (read-line stream eofp eof)
        (read-line stream eofp eof)
        (read-line stream eofp eof)))

(defun read-dtrace-backtrace (stream &optional eofp eof)
  (loop for line = (read-line stream nil :eof)
     when (eq line :eof)
     do (if eofp
            (error "End of file encountered")
            (return-from read-dtrace-backtrace eof))
     until (= (length line) 0)
     collect line))

(defun write-block (stream backtrace)
  (loop for x in backtrace
     do (princ x stream)
     do (terpri stream))
  (terpri stream))

(defun cleanup-frame (line)
  (let (pos)
    (cond
      ((setf pos (search "cclasp-boehm-image.fasl`" line))
       (concatenate 'string (subseq line 0 pos) (subseq line (+ pos #.(length "cclasp-boehm-image.fasl`")) (length line))))
      ((setf pos (search "iclasp-boehm`" line))
       (concatenate 'string (subseq line 0 pos) (subseq line (+ pos #.(length "iclasp-boehm`")) (length line))))
      (t line))))

(defun prune-backtrace (backtrace)
  (declare (optimize speed))
  (list*
   (cleanup-frame (car backtrace))
   (loop for line in (cdr backtrace)
      unless (search "VariadicFunctor" line)
      unless (search "core__call_with_variable_bound" line)
      unless (search "core::apply_method" line)
      unless (search "core::funcall_va_list" line)
      unless (search "core::funcall_consume_valist" line)
      unless (search "core::cl__apply" line)
      unless (search "FuncallableInstance_O::entry_point" line)
      unless (search "standard_dispatch" line)
      unless (search "funcall_frame" line)
      unless (search "cc_call_multipleValueOneFormCall" line)
      unless (search "core::core__funwind_protect" line)
      unless (search "core::core__multiple_value_prog1_function" line)
      unless (search "LAMBDA^^COMMON-LISP_FN" line)
      unless (search "COMBINE-METHOD-FUNCTIONS3.LAMBDA" line)
      collect (cleanup-frame line))))

(defun prune-dtrace-log (input output &key (verbose t))
  (let ((fin (open input :direction :input :external-format :latin-1))
        (fout (open output :direction :output :if-exists :supersede :external-format :latin-1))
        (count 0))
    (let ((header (read-dtrace-header fin)))
      (write-block fout header)
      (loop for backtrace = (read-dtrace-backtrace fin nil :eof)
         until (eq backtrace :eof)
         for pruned = (prune-backtrace backtrace)
         when (> (length backtrace) 4000)
         do (progn
              (format t "------------ input file pos: ~a~%" (file-position fin))
              (format t "Backtrace with ~a frames~%" (length backtrace))
              (format t "~a~%" pruned #++(last backtrace 5)))
         do (write-block fout pruned)
         do (incf count)))
    (when verbose (format t "Pruned ~a stacks~%" count))
    (close fin)
    (close fout)))

(let ((in-file (third sb-ext:*posix-argv*))
      (out-file (fourth sb-ext:*posix-argv*)))
  (format t "prune ~a -> ~a~%" in-file out-file)
  (prune-dtrace-log in-file out-file))
