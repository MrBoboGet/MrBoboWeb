#include <vector>
#include <string>
#include <MBUtility/MBInterfaces.h>
#include <MBUtility/IndeterminateInputStream.h>
#include <memory>


namespace MBSystem
{
    class BiDirectionalSubProcess : public MBUtility::MBOctetOutputStream,public MBUtility::IndeterminateInputStream
    {
    private:
        //Made so that the underlying system implementations can use whatever data they want, without 
        //cluttering the include with system definitions 
        std::unique_ptr<void,void(*)(void*)> m_ImplementationData = std::unique_ptr<void,void(*)(void*)>(nullptr,[](void*) -> void {});
    public:
        //Uses path, inherits environment
        BiDirectionalSubProcess(std::string const& ProgramName,std::vector<std::string> const& Args);
        
        size_t Write(void const* Data,size_t DataSize) override;
        void Flush() override;
        size_t ReadSome(void* Buffer,size_t BufferSize) override;
       
        //should clean up all state if a program was succesfully allocated, should not throw exception
        void Close();
        ~BiDirectionalSubProcess()
        {
            Close();   
        }
    };
}
