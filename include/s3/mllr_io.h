int32
store_reg_mat (const char    *regmatfn,
	       const uint32  *veclen,
	       uint32  n_class,
	       uint32  n_stream,
	       float32 ****A,
	       float32 ***B);


int32
read_reg_mat (const char   *regmatfn,
	      const uint32  **veclen,
	      uint32  *n_class,
	      uint32  *n_stream,
	      float32 *****A,
	      float32 ****B);
