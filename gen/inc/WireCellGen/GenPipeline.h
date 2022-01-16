/**
    Define a pipeline facade over WCT nodes.

    FIXME: this needs to move into the mithical "interface utilities"
    library along with much other stuff.
 */

#ifndef WIRECELL_GEN_PIPELINE
#define WIRECELL_GEN_PIPELINE

#include "WireCellIface/IQueuedoutNode.h"
#include "WireCellIface/IFunctionNode.h"
#include "WireCellIface/ISourceNode.h"

#include <queue>
#include <vector>

namespace WireCell {

    typedef std::queue<boost::any> Pipe;

    // Base class for something that executes as a thunk.
    class Proc {
       public:
        virtual ~Proc() {}
        // Execute one operation of the process.
        virtual bool operator()() = 0;
    };

    typedef std::vector<Proc*> Pipeline;

    // Base class for something that pushes elements to a Pipe.
    class SourceProc : public virtual Proc {
       public:
        virtual ~SourceProc() {}
        // Return output Pipe
        virtual Pipe& output_pipe() = 0;
    };
    // Base class for something that pops elements from a Pipe.
    class SinkProc : public virtual Proc {
       public:
        virtual ~SinkProc() {}
        // Return output Pipe
        virtual Pipe& input_pipe() = 0;
    };

    // Base class for something that intermediates between two pipes
    class FilterProc : public virtual SourceProc, public virtual SinkProc {
       public:
        virtual ~FilterProc() {}
    };

    // A filter pops one element from input pipe and pushes it to output
    // pipe each execution.
    class ShuntProc : public FilterProc {
       public:
        ShuntProc(Pipe& iq, Pipe& oq)
          : iq(iq)
          , oq(oq)
        {
        }
        virtual ~ShuntProc() {}

        virtual bool operator()()
        {
            if (iq.empty()) {
                return false;
            }
            oq.push(iq.front());
            iq.pop();
            return true;
        }
        virtual Pipe& input_pipe() { return iq; }
        virtual Pipe& output_pipe() { return oq; }

       private:
        Pipe& iq;
        Pipe& oq;
    };

    // A proc which pops an input element, feeds it to a node and pushes the result.
    class FunctionNodeProc : public FilterProc {
       public:
        typedef typename WireCell::IFunctionNodeBase node_t;
        typedef std::shared_ptr<node_t> node_pointer_t;

        FunctionNodeProc(node_pointer_t node)
          : node(node)
        {
        }
        virtual ~FunctionNodeProc() {}

        virtual Pipe& input_pipe() { return iq; }
        virtual Pipe& output_pipe() { return oq; }

        virtual bool operator()()
        {
            if (iq.empty()) {
                return false;
            }
            boost::any anyout;
            bool ok = (*node)(iq.front(), anyout);
            if (!ok) return false;
            iq.pop();
            oq.push(anyout);
            return true;
        }

       private:
        Pipe iq, oq;
        node_pointer_t node;
    };

    // A sink proc that pops and drops
    class DropSinkProc : public SinkProc {
       public:
        DropSinkProc() {}
        virtual ~DropSinkProc() {}

        virtual bool operator()()
        {
            if (iq.empty()) {
                return false;
            }
            iq.pop();
            return true;
        }
        virtual Pipe& input_pipe() { return iq; }

       private:
        Pipe iq;
    };

    // A proc which produces one element using a node
    class SourceNodeProc : public SourceProc {
       public:
        typedef typename WireCell::ISourceNodeBase node_t;
        typedef std::shared_ptr<node_t> node_pointer_t;

        SourceNodeProc(node_pointer_t node)
          : node(node)
        {
        }
        virtual ~SourceNodeProc() {}

        virtual Pipe& output_pipe() { return oq; }

        virtual bool operator()()
        {
            boost::any anyout;
            bool ok = (*node)(anyout);
            if (!ok) return false;
            oq.push(anyout);
            return true;
        }

       private:
        Pipe oq;
        node_pointer_t node;
    };

    // A proc which pops gives next element a node and pops it if node does puke.
    class SinkNodeProc : public SinkProc {
       public:
        typedef typename WireCell::ISinkNodeBase node_t;
        typedef std::shared_ptr<node_t> node_pointer_t;

        SinkNodeProc(node_pointer_t node)
          : node(node)
        {
        }
        virtual ~SinkNodeProc() {}

        virtual Pipe& input_pipe() { return iq; }

        virtual bool operator()()
        {
            if (iq.empty()) {
                return false;
            }
            bool ok = (*node)(iq.front());
            if (!ok) return false;
            iq.pop();
            return true;
        }

       private:
        Pipe iq;
        node_pointer_t node;
    };

    typedef std::deque<boost::any> queuedany;

    // A proc which pops an input element, feeds it to a node, assumes the
    // node is a queuedany, iterates over it and feeds individual elements
    // to the output queue.
    class QueuedNodeProc : public FilterProc {
       public:
        typedef typename WireCell::IQueuedoutNodeBase node_t;
        typedef std::shared_ptr<node_t> node_pointer_t;

        QueuedNodeProc(node_pointer_t node)
          : node(node)
        {
        }
        virtual ~QueuedNodeProc() {}

        virtual Pipe& input_pipe() { return iq; }
        virtual Pipe& output_pipe() { return oq; }

        virtual bool operator()()
        {
            if (iq.empty()) {
                return false;
            }
            queuedany anyq;
            bool ok = (*node)(iq.front(), anyq);
            if (!ok) return false;
            iq.pop();
            for (auto anyo : anyq) {
                oq.push(anyo);
            }
            return true;
        }

       private:
        Pipe iq, oq;
        node_pointer_t node;
    };

    Proc* join(Pipeline& pipeline, Proc* src, Proc* dst)
    {
        pipeline.push_back(src);
        auto link =
            new ShuntProc(dynamic_cast<SourceProc*>(src)->output_pipe(), dynamic_cast<SinkProc*>(dst)->input_pipe());
        pipeline.push_back(link);

        // do not push dst.  return just for syntactic sugar
        return dst;
    }

}  // namespace WireCell
#endif
