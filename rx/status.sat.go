
ns rx
  interface Status
    Code type uint8_t
    Message type const std::string&
    OK Status  // == OK (no error)
    Status(Code, Message) (Status)
    ok()      bool
    code()    Code
    message() Message
    =(Status) Status
  

  type Status
    :OK()
    state char[]

    Status(code Code, msg Message) state(1 + msg.size() + 1)
      @state[0] = code
      memcpy(@state + 1, msg.data(), msg.size())
      @state[1 + msg.size()] = 0

    ok()      { !_state }
    code()    { if _state _state[0] else 0 }
    message() { if _state &_state[1] else "" }  // `if A B else C` is encoded as `A ? B : C`




  test()
    st = Status(123, "Hello")
    if st == Status.OK
      console.log "status #{st} is OK"
    else
      console.log "status #{st} is not OK"


