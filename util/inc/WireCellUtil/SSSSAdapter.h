#ifndef WIRECELL_SSSSADAPTER
#define WIRECELL_SSSSADAPTER

namespace WireCell {

    /** An adapter which drives the push-pull sink/process/source
     * paradigm to the pure-pull signal/slot execution model.
     */
    template <typename InputType, typename OutputType, class Processor>
    class SigSlotSinkSourceAdapter {
       public:
        typedef boost::signals2::signal<InputType()> signal_type;
        typedef signal_type::slot_type input_slot_type;

        SigSlotSinkSourceAdapter(Processor& proc)
          : m_proc(proc)
        {
        }

        OutputType operator()()
        {
            InputType input = m_proc();
            m_proc.sink(input);

            OutputType ot;
            m_proc.source(ot);
            return ot;
        }

        void connect(const input_slot_type& s) { m_input.connect(s); }

       private:
        Processor& m_proc;
        signal_type m_input;
    };

}  // namespace WireCell
#endif
