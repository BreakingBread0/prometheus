#pragma once

#ifdef _KERNEL_MODE
#include <ntimage.h>
#else
#include <winnt.h>
#endif



namespace Pe
{



    // To avoid dependence on <type_traits>
    namespace tr
    {
        template <typename>
        constexpr bool is_lvalue_reference_v = false;

        template <typename T>
        constexpr bool is_lvalue_reference_v<T&> = true;

        template <typename T>
        struct remove_reference
        {
            using type = T;
        };

        template <typename T>
        struct remove_reference<T&>
        {
            using type = T;
        };

        template <typename T>
        struct remove_reference<T&&>
        {
            using type = T;
        };

        template <typename T>
        using remove_reference_t = typename remove_reference<T>::type;

        template <typename T>
        [[nodiscard]] constexpr T&& forward(remove_reference_t<T>& arg) noexcept
        {
            return static_cast<T&&>(arg);
        }

        template <class T>
        [[nodiscard]] constexpr T&& forward(remove_reference_t<T>&& arg) noexcept
        {
            static_assert(!is_lvalue_reference_v<T>, "bad forward call");
            return static_cast<T&&>(arg);
        }
    } // namespace tr



    enum class Arch : unsigned char
    {
        unknown,
        x32,
        x64,
        native = (sizeof(size_t) == sizeof(unsigned long) ? x32 : x64),
        inverse = (native == x32 ? x64 : x32)
    };

    enum class ImgType : unsigned char
    {
        file,
        module
    };

    using Rva = unsigned int;
    using Ordinal = unsigned short;

    enum class ImportType
    {
        unknown,
        name,
        ordinal
    };

    enum class ExportType
    {
        unknown,
        exact,
        forwarder
    };

    enum class RelocType
    {
        unknown,
        absolute,
        high,
        low,
        highlow,
        highadj,
        dir64
    };

    struct Reloc
    {
        unsigned short offsetInPage : 12;
        unsigned short rawType : 4; // IMG_REL_BASED_***

        RelocType type() const noexcept
        {
            switch (rawType)
            {
            case IMAGE_REL_BASED_ABSOLUTE: return RelocType::absolute;
            case IMAGE_REL_BASED_HIGH: return RelocType::high;
            case IMAGE_REL_BASED_LOW: return RelocType::low;
            case IMAGE_REL_BASED_HIGHLOW: return RelocType::highlow;
            case IMAGE_REL_BASED_HIGHADJ: return RelocType::highadj;
            case IMAGE_REL_BASED_DIR64: return RelocType::dir64;
            default:
                return RelocType::unknown;
            }
        }
    };
    static_assert(sizeof(Reloc) == sizeof(unsigned short), "Invalid size of Reloc");

    struct GenericTypes
    {
        using DosHeader = IMAGE_DOS_HEADER;
        using ImgDataDir = IMAGE_DATA_DIRECTORY;
        using SecHeader = IMAGE_SECTION_HEADER;
        using ImgImportByName = IMAGE_IMPORT_BY_NAME;
        using BoundForwarderRef = IMAGE_BOUND_FORWARDER_REF;

        struct RUNTIME_FUNCTION // For x86 headers compatibility
        {
            unsigned int BeginAddress;
            unsigned int EndAddress;
            union
            {
                unsigned int UnwindInfoAddress;
                unsigned int UnwindData;
            } UnwindInfo;
        };

        union ExportAddressTableEntry
        {
            Rva address;
            Rva forwarderString;
        };

        struct RelocsTable
        {
            struct Header
            {
                Rva pageRva;
                unsigned int relocsSizeInBytes;
            } hdr;

            Reloc relocs[1];

            unsigned int count() const noexcept
            {
                return hdr.relocsSizeInBytes / sizeof(Reloc);
            }
        };

        using FnImageTlsCallback = PIMAGE_TLS_CALLBACK;
    };

    template <Arch arch>
    struct Types;

    template <>
    struct Types<Arch::x32> : public GenericTypes
    {
        using NtHeaders = IMAGE_NT_HEADERS32;
        using OptHeader = IMAGE_OPTIONAL_HEADER32;
        using ImgThunkData = IMAGE_THUNK_DATA32;
        union ImportAddressTableEntry
        {
            unsigned int raw;
            ImgThunkData thunk;
            struct
            {
                Rva hintNameRva : 31;
            } name;
            struct
            {
                unsigned int ord : 16;
            } ordinal;
            struct {
                unsigned int reserved : 31;
                unsigned int importByOrdinal : 1;
            } importType;

            bool valid() const noexcept
            {
                return raw != 0;
            }

            ImportType type() const noexcept
            {
                if (!valid())
                {
                    return ImportType::unknown;
                }
                return importType.importByOrdinal ? ImportType::ordinal : ImportType::name;
            }
        };
        static_assert(sizeof(ImportAddressTableEntry) == sizeof(unsigned int), "Invalid size of ImportAddressTableEntry");
        using ImportLookupTableEntry = ImportAddressTableEntry;
        using ImportNameTableEntry = ImportAddressTableEntry;

        using TlsDir = IMAGE_TLS_DIRECTORY32;

        static constexpr auto k_magic = 0x010Bu; // PE32
    };

    template <>
    struct Types<Arch::x64> : public GenericTypes
    {
        using NtHeaders = IMAGE_NT_HEADERS64;
        using OptHeader = IMAGE_OPTIONAL_HEADER64;
        using ImgThunkData = IMAGE_THUNK_DATA64;
        union ImportAddressTableEntry
        {
            unsigned long long raw;
            ImgThunkData thunk;
            struct
            {
                Rva hintNameRva : 31;
            } name;
            struct
            {
                unsigned long long ord : 16;
            } ordinal;
            struct {
                unsigned long long reserved : 63;
                unsigned long long importByOrdinal : 1;
            } importType;

            bool valid() const noexcept
            {
                return raw != 0;
            }

            ImportType type() const noexcept
            {
                if (!valid())
                {
                    return ImportType::unknown;
                }
                return importType.importByOrdinal ? ImportType::ordinal : ImportType::name;
            }
        };
        static_assert(sizeof(ImportAddressTableEntry) == sizeof(unsigned long long), "Invalid size of ImportAddressTableEntry");
        using ImportLookupTableEntry = ImportAddressTableEntry;
        using ImportNameTableEntry = ImportAddressTableEntry;

        using TlsDir = IMAGE_TLS_DIRECTORY64;

        static constexpr auto k_magic = 0x020Bu; // PE32+
    };

    template <typename DirType, unsigned int id>
    struct Dir
    {
        using Type = DirType;
        static constexpr auto k_id = id;
    };

    using DirImports = Dir<IMAGE_IMPORT_DESCRIPTOR, IMAGE_DIRECTORY_ENTRY_IMPORT>;
    using DirDelayedImports = Dir<IMAGE_DELAYLOAD_DESCRIPTOR, IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT>;
    using DirBoundImports = Dir<IMAGE_BOUND_IMPORT_DESCRIPTOR, IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT>;
    using DirExports = Dir<IMAGE_EXPORT_DIRECTORY, IMAGE_DIRECTORY_ENTRY_EXPORT>;
    using DirRelocs = Dir<IMAGE_BASE_RELOCATION, IMAGE_DIRECTORY_ENTRY_BASERELOC>;
    using DirExceptions = Dir<GenericTypes::RUNTIME_FUNCTION, IMAGE_DIRECTORY_ENTRY_EXCEPTION>;
    using DirDebug = Dir<IMAGE_DEBUG_DIRECTORY, IMAGE_DIRECTORY_ENTRY_DEBUG>;

    template <Arch arch>
    using DirTls = Dir<typename Types<arch>::TlsDir, IMAGE_DIRECTORY_ENTRY_TLS>;

    template <typename Dir>
    struct DirectoryDescriptor
    {
        using DirInfo = Dir;

        const typename Dir::Type* ptr;
        unsigned long size;

        bool valid() const noexcept
        {
            return ptr && size;
        }

        bool empty() const noexcept
        {
            return !valid();
        }
    };

    class Sections; // Arch-independent
    template <Arch> class Imports;
    template <Arch> class DelayedImports;
    template <Arch> class BoundImports;
    template <Arch> class Exports;
    template <Arch> class Relocs;
    template <Arch> class Exceptions;
    template <Arch> class Tls;
    template <Arch> class Debug;


    struct PeMagic
    {
        static constexpr uint16_t k_mz = 0x5A4D; // MZ
        static constexpr uint32_t k_pe = 0x00004550; // "PE\0\0"
    };

    template <Arch arch>
    class PeHeaders : public PeMagic
    {
    public:
        using Types = Types<arch>;
        using DosHeader = typename Types::DosHeader;
        using NtHeaders = typename Types::NtHeaders;
        using OptHeader = typename Types::OptHeader;

    public:
        static constexpr auto k_magic = Types::k_magic;

    private:
        const void* const m_base;

    public:
        explicit PeHeaders(const void* const base) noexcept : m_base(base)
        {
        }

        const DosHeader* dos() const noexcept
        {
            return static_cast<const DosHeader*>(m_base);
        }

        const NtHeaders* nt() const noexcept
        {
            return reinterpret_cast<const NtHeaders*>(static_cast<const unsigned char*>(m_base) + dos()->e_lfanew);
        }

        const OptHeader* opt() const noexcept
        {
            return &nt()->OptionalHeader;
        }

        const void* mod() const noexcept
        {
            return m_base;
        }

        bool valid() const noexcept
        {
            const auto* const dosHdr = dos();
            if (!dosHdr)
            {
                return false;
            }

            if (dosHdr->e_magic != k_mz)
            {
                return false;
            }

            const auto* const ntHdr = nt();
            if (ntHdr->Signature != k_pe)
            {
                return false;
            }

            const auto* const optHdr = opt();
            if (optHdr->Magic != k_magic)
            {
                return false;
            }

            return true;
        }
    };

    struct PeArch
    {
        static Arch classify(const void* const base) noexcept
        {
            if (PeHeaders<Arch::native>(base).valid())
            {
                return Arch::native;
            }
            else if (PeHeaders<Arch::inverse>(base).valid())
            {
                return Arch::inverse;
            }
            else
            {
                return Arch::unknown;
            }
        }
    };


    struct Align
    {
        template <typename Type>
        static constexpr Type alignDown(const Type value, const Type factor) noexcept
        {
            return value & ~(factor - 1);
        }

        template <typename Type>
        static constexpr Type alignUp(const Type value, const Type factor) noexcept
        {
            return alignDown<Type>(value - 1, factor) + factor;
        }
    };


    template <Arch arch>
    class Pe
    {
    public:
        using ImgDataDir = typename GenericTypes::ImgDataDir;

    private:
        const void* const m_base;
        const ImgType m_type;

    public:
        Pe(const ImgType type, const void* const base) noexcept : m_base(base), m_type(type)
        {
        }

        static Pe fromFile(const void* const buffer) noexcept
        {
            return Pe(ImgType::file, buffer);
        }

        static Pe fromModule(const void* const base) noexcept
        {
            return Pe(ImgType::module, base);
        }

        PeHeaders<arch> headers() const noexcept
        {
            return PeHeaders<arch>(m_base);
        }

        template <typename Type>
        const Type* byRva(const Rva rva) const noexcept
        {
            if (m_type == ImgType::module)
            {
                return reinterpret_cast<const Type*>(static_cast<const unsigned char*>(m_base) + rva);
            }

            const auto* const optHdr = headers().opt();
            const auto fileAlignment = optHdr->FileAlignment;
            const auto sectionAlignment = optHdr->SectionAlignment;

            constexpr auto k_minimalSectionAlignment = 512u;
            for (const auto& sec : sections())
            {
                const auto sizeOnDisk = sec.SizeOfRawData;
                const auto sizeInMem = sec.Misc.VirtualSize;

                unsigned long long sectionBase = 0;
                unsigned long long sectionSize = 0;
                unsigned long long sectionOffset = 0;
                if (sectionAlignment >= k_minimalSectionAlignment)
                {
                    sectionBase = Align::alignDown<unsigned long long>(sec.VirtualAddress, sectionAlignment);
                    const auto alignedFileSize = Align::alignUp<unsigned long long>(sizeOnDisk, fileAlignment);
                    const auto alignedSectionSize = Align::alignUp<unsigned long long>(sizeInMem, sectionAlignment);
                    sectionSize = (alignedFileSize > alignedSectionSize) ? alignedSectionSize : alignedFileSize;
                    sectionOffset = Align::alignDown<unsigned long long>(sec.PointerToRawData, k_minimalSectionAlignment);
                }
                else
                {
                    sectionBase = sec.VirtualAddress;
                    sectionSize = (sizeOnDisk > sizeInMem) ? sizeInMem : sizeOnDisk;
                    sectionOffset = sec.PointerToRawData;
                }

                if ((rva >= sectionBase) && (rva < sectionBase + sectionSize))
                {
                    return reinterpret_cast<const Type*>(static_cast<const unsigned char*>(m_base) + (sectionOffset + (rva - sectionBase)));
                }
            }

            return nullptr;
        }

        template <typename Type>
        const Type* byOffset(const unsigned int offset) const noexcept
        {
            return reinterpret_cast<const Type*>(reinterpret_cast<const unsigned char*>(m_base) + offset);
        }

        const ImgDataDir* directory(const unsigned int id) const noexcept
        {
            return &headers().opt()->DataDirectory[id];
        }

        template <typename DirType>
        DirectoryDescriptor<DirType> directory() const noexcept
        {
            const auto* const directoryHeader = directory(DirType::k_id);
            if (!directoryHeader->Size)
            {
                return {};
            }

            return DirectoryDescriptor<DirType>
            {
                byRva<typename DirType::Type>(directoryHeader->VirtualAddress),
                    directoryHeader->Size
            };
        }

        unsigned long long imageBase() const noexcept
        {
            return headers().opt()->ImageBase;
        }

        unsigned long imageSize() const noexcept
        {
            return headers().opt()->SizeOfImage;
        }

        unsigned long long entryPoint() const noexcept
        {
            return static_cast<unsigned long long>(reinterpret_cast<size_t>(byRva<void>(headers().opt()->AddressOfEntryPoint)));
        }

        ImgType type() const noexcept
        {
            return m_type;
        }

        bool valid() const noexcept
        {
            return headers().valid();
        }

        Sections sections() const noexcept;
        Imports<arch> imports() const noexcept;
        DelayedImports<arch> delayedImports() const noexcept;
        BoundImports<arch> boundImports() const noexcept;
        Exports<arch> exports() const noexcept;
        Relocs<arch> relocs() const noexcept;
        Exceptions<arch> exceptions() const noexcept;
        Tls<arch> tls() const noexcept;
        Debug<arch> debug() const noexcept;
    };

    using Pe32 = Pe<Arch::x32>;
    using Pe64 = Pe<Arch::x64>;
    using PeNative = Pe<Arch::native>;



    class Sections
    {
    public:
        class Iterator
        {
        private:
            const Sections& m_owner;
            unsigned int m_pos;

        public:
            Iterator(const Sections& owner, const unsigned int pos) noexcept
                : m_owner(owner)
                , m_pos(pos)
            {
                if (m_pos > m_owner.count())
                {
                    m_pos = m_owner.count();
                }
            }

            Iterator& operator ++ () noexcept
            {
                if (m_pos < m_owner.count())
                {
                    ++m_pos;
                }

                return *this;
            }

            Iterator operator ++ (int) noexcept
            {
                const auto it = *this;
                ++(*this);
                return it;
            }

            bool operator == (const Iterator& it) const noexcept
            {
                return m_pos == it.m_pos;
            }

            bool operator != (const Iterator& it) const noexcept
            {
                return !operator == (it);
            }

            const typename GenericTypes::SecHeader& operator * () const noexcept
            {
                return *operator -> ();
            }

            const typename GenericTypes::SecHeader* operator -> () const noexcept
            {
                return &m_owner.sections()[m_pos];
            }
        };

    private:
        const typename GenericTypes::SecHeader* const m_sections;
        const unsigned int m_count;

    public:
        Sections(const typename GenericTypes::SecHeader* const sections, const unsigned int count) noexcept
            : m_sections(sections)
            , m_count(count)
        {
        }

        const typename GenericTypes::SecHeader* sections() const noexcept
        {
            return m_sections;
        }

        bool valid() const noexcept
        {
            return m_sections != nullptr;
        }

        bool empty() const noexcept
        {
            return !valid() || !m_count;
        }

        unsigned int count() const noexcept
        {
            return m_count;
        }

        Iterator begin() const noexcept
        {
            return Iterator(*this, 0);
        }

        Iterator end() const noexcept
        {
            return Iterator(*this, m_count);
        }
    };



    template <typename Object>
    class Iterator
    {
    public:
        struct TheEnd
        {
        };

    private:
        Object m_object;

    public:
        template <typename... Args>
        Iterator(Args&&... args) noexcept : m_object(tr::forward<Args>(args)...)
        {
        }

        explicit Iterator(const Object& obj) noexcept : m_object(obj)
        {
        }

        bool operator == (const Iterator& it) const noexcept
        {
            return m_object == it.m_object;
        }

        bool operator == (TheEnd) const noexcept
        {
            return m_object == TheEnd{};
        }

        bool operator != (const Iterator& it) const noexcept
        {
            return !operator == (it);
        }

        bool operator != (TheEnd) const noexcept
        {
            return !operator == (TheEnd{});
        }

        Iterator& operator ++ () noexcept
        {
            ++m_object;
            return *this;
        }

        Iterator operator ++ (int) noexcept
        {
            const auto prev = *this;
            ++(*this);
            return prev;
        }

        const Object& operator * () const noexcept
        {
            return m_object;
        }

        Object& operator * () noexcept
        {
            return m_object;
        }

        const Object* operator -> () const noexcept
        {
            return &m_object;
        }

        Object* operator -> () noexcept
        {
            return &m_object;
        }
    };



    template <Arch arch>
    class Imports
    {
    public:
        class ModuleEntry;

        class FunctionEntry
        {
        private:
            const ModuleEntry& m_lib;
            unsigned int m_index;

        public:
            FunctionEntry(const ModuleEntry& lib, const unsigned int index) noexcept : m_lib(lib), m_index(index)
            {
            }

            const ModuleEntry& lib() const noexcept
            {
                return m_lib;
            }

            unsigned int index() const noexcept
            {
                return m_index;
            }

            const typename Types<arch>::ImportAddressTableEntry* importAddressTableEntry() const noexcept // Import Address Table
            {
                return &m_lib.importAddressTable()[m_index];
            }

            const typename Types<arch>::ImportLookupTableEntry* importLookupTableEntry() const noexcept // Import Lookup Table
            {
                return &m_lib.importLookupTable()[m_index];
            }

            bool valid() const noexcept
            {
                return importLookupTableEntry()->valid();
            }

            ImportType type() const noexcept
            {
                return importLookupTableEntry()->type();
            }

            const typename GenericTypes::ImgImportByName* name() const noexcept
            {
                if (type() != ImportType::name)
                {
                    return nullptr;
                }

                const Rva rva = importLookupTableEntry()->name.hintNameRva;
                return m_lib.pe().template byRva<typename GenericTypes::ImgImportByName>(rva);
            }

            unsigned long long address() const noexcept
            {
                if ((m_lib.pe().type() == ImgType::file) && !m_lib.bound())
                {
                    return 0;
                }

                return importAddressTableEntry()->raw;
            }

            unsigned short ordinal() const noexcept
            {
                if (type() != ImportType::ordinal)
                {
                    return 0;
                }

                return importLookupTableEntry()->ordinal.ord;
            }

            bool operator == (const FunctionEntry& entry) const noexcept
            {
                return index() == entry.index();
            }

            bool operator == (typename Iterator<FunctionEntry>::TheEnd) const noexcept
            {
                return !valid();
            }

            FunctionEntry& operator ++ () noexcept
            {
                ++m_index;
                return *this;
            }
        };

        using FunctionIterator = Iterator<FunctionEntry>;

        class ModuleEntry
        {
        private:
            const Pe<arch>& m_pe;
            const typename DirImports::Type* m_descriptor;

        public:
            ModuleEntry(const Pe<arch>& pe, const typename DirImports::Type* const descriptor) noexcept
                : m_pe(pe)
                , m_descriptor(descriptor)
            {
            }

            const Pe<arch>& pe() const noexcept
            {
                return m_pe;
            }

            const typename DirImports::Type* descriptor() const noexcept
            {
                return m_descriptor;
            }

            bool valid() const noexcept
            {
                return m_descriptor && m_descriptor->Characteristics;
            }

            const char* libName() const noexcept
            {
                return m_pe.template byRva<char>(m_descriptor->Name);
            }

            // Import Address Table:
            const typename Types<arch>::ImportAddressTableEntry* importAddressTable() const noexcept
            {
                return m_pe.template byRva<typename Types<arch>::ImportAddressTableEntry>(m_descriptor->FirstThunk);
            }

            // Import Lookup Table:
            const typename Types<arch>::ImportLookupTableEntry* importLookupTable() const noexcept
            {
                return m_pe.template byRva<typename Types<arch>::ImportLookupTableEntry>(m_descriptor->OriginalFirstThunk);
            }

            bool bound() const noexcept
            {
                return descriptor()->TimeDateStamp != 0;
            }

            bool operator == (const ModuleEntry& entry) const noexcept
            {
                return descriptor() == entry.descriptor();
            }

            bool operator == (typename Iterator<ModuleEntry>::TheEnd) const noexcept
            {
                return !valid();
            }

            ModuleEntry& operator ++ () noexcept
            {
                ++m_descriptor;
                return *this;
            }

            FunctionIterator begin() const noexcept
            {
                return FunctionIterator(*this, 0);
            }

            typename FunctionIterator::TheEnd end() const noexcept
            {
                return {};
            }
        };

        using ModuleIterator = Iterator<ModuleEntry>;


    private:
        const Pe<arch>& m_pe;

    public:
        explicit Imports(const Pe<arch>& pe) noexcept : m_pe(pe)
        {
        }

        const Pe<arch>& pe() const noexcept
        {
            return m_pe;
        }

        DirectoryDescriptor<DirImports> descriptor() const noexcept
        {
            return m_pe.template directory<DirImports>();
        }

        bool valid() const noexcept
        {
            return descriptor().valid();
        }

        bool empty() const noexcept
        {
            const auto importDescriptor = descriptor();
            return importDescriptor.empty() || !importDescriptor.ptr->FirstThunk;
        }

        ModuleIterator begin() const noexcept
        {
            return ModuleIterator(m_pe, descriptor().ptr);
        }

        typename ModuleIterator::TheEnd end() const noexcept
        {
            return {};
        }
    };



    template <Arch arch>
    class DelayedImports
    {
    public:
        class ModuleEntry;

        class FunctionEntry
        {
        private:
            const ModuleEntry& m_lib;
            unsigned int m_index;

        public:
            FunctionEntry(const ModuleEntry& lib, const unsigned int index) noexcept : m_lib(lib), m_index(index)
            {
            }

            const ModuleEntry& lib() const noexcept
            {
                return m_lib;
            }

            unsigned int index() const noexcept
            {
                return m_index;
            }

            const typename Types<arch>::ImportAddressTableEntry* importAddressTableEntry() const noexcept
            {
                return &m_lib.importAddressTable()[m_index];
            }

            const typename Types<arch>::ImportNameTableEntry* importNameTableEntry() const noexcept
            {
                return &m_lib.importNameTable()[m_index];
            }

            bool valid() const noexcept
            {
                return importNameTableEntry()->valid();
            }

            ImportType type() const noexcept
            {
                return importNameTableEntry()->type();
            }

            const typename GenericTypes::ImgImportByName* name() const noexcept
            {
                if (type() != ImportType::name)
                {
                    return nullptr;
                }

                const Rva rva = importNameTableEntry()->name.hintNameRva;
                return m_lib.pe().template byRva<typename GenericTypes::ImgImportByName>(rva);
            }

            unsigned long long address() const noexcept
            {
                return importAddressTableEntry()->raw;
            }

            unsigned int ordinal() const noexcept
            {
                if (type() != ImportType::ordinal)
                {
                    return 0;
                }

                return importNameTableEntry()->ordinal.ord;
            }

            bool operator == (const FunctionEntry& entry) const noexcept
            {
                return index() == entry.index();
            }

            bool operator == (typename Iterator<FunctionEntry>::TheEnd) const noexcept
            {
                return !valid();
            }

            FunctionEntry& operator ++ () noexcept
            {
                ++m_index;
                return *this;
            }
        };

        using FunctionIterator = Iterator<FunctionEntry>;

        class ModuleEntry
        {
        private:
            const Pe<arch>& m_pe;
            const typename DirDelayedImports::Type* m_descriptor;

        public:
            ModuleEntry(const Pe<arch>& pe, const typename DirDelayedImports::Type* const descriptor) noexcept
                : m_pe(pe)
                , m_descriptor(descriptor)
            {
            }

            const Pe<arch>& pe() const noexcept
            {
                return m_pe;
            }

            const typename DirDelayedImports::Type* descriptor() const noexcept
            {
                return m_descriptor;
            }

            bool valid() const noexcept
            {
                return m_descriptor && m_descriptor->DllNameRVA;
            }

            const char* moduleName() const noexcept
            {
                return m_pe.template byRva<char>(m_descriptor->DllNameRVA);
            }

            // Import Address Table:
            const typename Types<arch>::ImportAddressTableEntry* importAddressTable() const noexcept
            {
                return m_pe.template byRva<typename Types<arch>::ImportAddressTableEntry>(m_descriptor->ImportAddressTableRVA);
            }

            // Import Name Table:
            const typename Types<arch>::ImportNameTableEntry* importNameTable() const noexcept
            {
                return m_pe.template byRva<typename Types<arch>::ImportNameTableEntry>(m_descriptor->ImportNameTableRVA);
            }

            bool operator == (const ModuleEntry& entry) const noexcept
            {
                return descriptor() == entry.descriptor();
            }

            bool operator == (typename Iterator<ModuleEntry>::TheEnd) const noexcept
            {
                return !valid();
            }

            ModuleEntry& operator ++ () noexcept
            {
                ++m_descriptor;
                return *this;
            }

            FunctionIterator begin() const noexcept
            {
                return FunctionIterator(*this, 0);
            }

            typename Iterator<ModuleEntry>::TheEnd end() const noexcept
            {
                return {};
            }
        };

        using ModuleIterator = Iterator<ModuleEntry>;

    private:
        const Pe<arch>& m_pe;

    public:
        explicit DelayedImports(const Pe<arch>& pe) noexcept : m_pe(pe)
        {
        }

        const Pe<arch>& pe() const noexcept
        {
            return m_pe;
        }

        DirectoryDescriptor<DirDelayedImports> descriptor() const noexcept
        {
            return m_pe.template directory<DirDelayedImports>();
        }

        bool valid() const noexcept
        {
            return descriptor().valid();
        }

        bool empty() const noexcept
        {
            const auto importDescriptor = descriptor();
            return importDescriptor.empty() || !importDescriptor.ptr->DllNameRVA;
        }

        ModuleIterator begin() const noexcept
        {
            return ModuleIterator(m_pe, descriptor().ptr);
        }

        typename ModuleIterator::TheEnd end() const noexcept
        {
            return {};
        }
    };



    template <Arch arch>
    class BoundImports
    {
    public:
        class ModuleEntry;

        class ForwarderEntry
        {
        private:
            const ModuleEntry& m_lib;
            unsigned int m_index;

        public:
            ForwarderEntry(const ModuleEntry& lib, const unsigned int index) noexcept : m_lib(lib), m_index(index)
            {
            }

            const ModuleEntry& lib() const noexcept
            {
                return m_lib;
            }

            unsigned int index() const noexcept
            {
                return m_index;
            }

            const typename GenericTypes::BoundForwarderRef* descriptor() const noexcept
            {
                return &m_lib.forwarders()[m_index];
            }

            bool valid() const noexcept
            {
                return descriptor()->OffsetModuleName != 0;
            }

            const char* libName() const noexcept
            {
                return reinterpret_cast<const char*>(m_lib.directoryBase()) + descriptor()->OffsetModuleName;
            }

            unsigned int timestamp() const noexcept
            {
                return descriptor()->TimeDateStamp;
            }

            bool operator == (const ForwarderEntry& entry) const noexcept
            {
                return index() == entry.index();
            }

            ForwarderEntry& operator ++ () noexcept
            {
                ++m_index;
                return *this;
            }
        };

        using ForwarderIterator = Iterator<ForwarderEntry>;


        class ModuleEntry
        {
        private:
            const typename DirBoundImports::Type* const m_directoryBase;
            const typename DirBoundImports::Type* m_descriptor;

        public:
            explicit ModuleEntry(const typename DirBoundImports::Type* const descriptor) noexcept
                : m_directoryBase(descriptor)
                , m_descriptor(descriptor)
            {
            }

            const typename DirBoundImports::Type* directoryBase() const noexcept
            {
                return m_directoryBase;
            }

            const typename DirBoundImports::Type* descriptor() const noexcept
            {
                return m_descriptor;
            }

            bool valid() const noexcept
            {
                return m_descriptor && m_descriptor->OffsetModuleName;
            }

            bool empty() const noexcept
            {
                return !valid() || !forwardersCount();
            }

            const char* libName() const noexcept
            {
                const auto offset = descriptor()->OffsetModuleName;
                if (!offset)
                {
                    return nullptr;
                }

                return reinterpret_cast<const char*>(m_directoryBase) + offset;
            }

            unsigned short forwardersCount() const noexcept
            {
                if (empty())
                {
                    return 0;
                }

                return descriptor()->NumberOfModuleForwarderRefs;
            }

            const typename GenericTypes::BoundForwarderRef* forwarders() const noexcept
            {
                if (empty())
                {
                    return nullptr;
                }

                return reinterpret_cast<const typename GenericTypes::BoundForwarderRef*>(descriptor() + 1);
            }

            bool operator == (const ModuleEntry& entry) const noexcept
            {
                return descriptor() == entry.descriptor();
            }

            bool operator == (typename Iterator<ForwarderEntry>::TheEnd) const noexcept
            {
                return !valid();
            }

            ForwarderEntry& operator ++ () noexcept
            {
                m_descriptor = reinterpret_cast<const typename DirBoundImports::Type*>(reinterpret_cast<const unsigned char*>(forwarders()) + forwardersCount() * sizeof(typename GenericTypes::BoundForwarderRef));
                return *this;
            }

            ForwarderIterator begin() const noexcept
            {
                return ForwarderIterator(*this, 0);
            }

            ForwarderIterator end() const noexcept
            {
                return ForwarderIterator(*this, forwardersCount());
            }
        };

        using ModuleIterator = Iterator<ModuleEntry>;

    private:
        const Pe<arch>& m_pe;

    public:
        explicit BoundImports(const Pe<arch>& pe) noexcept : m_pe(pe)
        {
        }

        const Pe<arch>& pe() const noexcept
        {
            return m_pe;
        }

        DirectoryDescriptor<DirBoundImports> descriptor() const noexcept
        {
            return m_pe.template directory<DirBoundImports>();
        }

        bool valid() const noexcept
        {
            return descriptor().valid() && descriptor().ptr->OffsetModuleName;
        }

        ModuleIterator begin() const noexcept
        {
            return ModuleIterator(descriptor().ptr);
        }

        typename ModuleIterator::TheEnd end() const noexcept
        {
            return {};
        }
    };



    template <Arch arch>
    class Exports
    {
    public:
        class FunctionEntry
        {
        private:
            const Exports& m_exports;
            const typename GenericTypes::ExportAddressTableEntry* const m_exportAddressTable;
            const Rva* m_name;
            const Ordinal* m_nameOrdinal;
            unsigned int m_index;

        public:
            FunctionEntry(const Exports& exports, const unsigned int index) noexcept
                : m_exports(exports)
                , m_exportAddressTable(exports.tables().exportAddressTable)
                , m_name(exports.tables().namePointerTable)
                , m_nameOrdinal(exports.tables().nameOrdinalTable)
                , m_index(index)
            {
            }

            unsigned int index() const noexcept
            {
                return m_index;
            }

            const typename GenericTypes::ExportAddressTableEntry* exportAddressTableEntry() const noexcept
            {
                return &m_exportAddressTable[m_index];
            }

            ExportType type() const noexcept
            {
                if (!valid())
                {
                    return ExportType::unknown;
                }

                return !m_exports.contains(exportAddressTableEntry()->forwarderString)
                    ? ExportType::exact
                    : ExportType::forwarder;
            }

            bool hasName() const noexcept
            {
                return m_index == *m_nameOrdinal;
            }

            const char* name() const noexcept
            {
                return hasName()
                    ? m_exports.pe().template byRva<char>(*m_name)
                    : nullptr;
            }

            unsigned int ordinal() const noexcept
            {
                return m_exports.ordinalBase() + m_index;
            }

            const void* address() const noexcept
            {
                if (type() != ExportType::exact)
                {
                    return nullptr;
                }

                return m_exports.pe().template byRva<void>(exportAddressTableEntry()->address);
            }

            const char* forwarder() const noexcept
            {
                if (type() != ExportType::forwarder)
                {
                    return nullptr;
                }

                return m_exports.pe().template byRva<char>(exportAddressTableEntry()->forwarderString);
            }

            bool valid() const noexcept
            {
                return m_index < m_exports.count();
            }

            bool operator == (const FunctionEntry& entry) const noexcept
            {
                return index() == entry.index();
            }

            FunctionEntry& operator ++ () noexcept
            {
                if (hasName())
                {
                    ++m_name;
                    ++m_nameOrdinal;
                }
                ++m_index;
                return *this;
            }
        };

        using FunctionIterator = Iterator<FunctionEntry>;

    public:
        struct Tables
        {
            const typename GenericTypes::ExportAddressTableEntry* exportAddressTable;
            const Rva* namePointerTable;
            const Ordinal* nameOrdinalTable;
        };

        class Export
        {
        private:
            union
            {
                const void* address;
                const char* forwarderName;
            } m_ptr;
            unsigned int m_ordinal;
            ExportType m_type;

        public:
            Export() noexcept : m_ptr{}, m_ordinal(0), m_type(ExportType::unknown)
            {
            }

            Export(const void* const addressOrForwarderName, const unsigned int ordinal, const ExportType type) noexcept
                : m_ptr{ addressOrForwarderName }
                , m_ordinal(ordinal)
                , m_type(type)
            {
            }

            const void* address() const noexcept
            {
                return (m_type == ExportType::exact)
                    ? m_ptr.address
                    : nullptr;
            }

            const char* forwarder() const noexcept
            {
                return (m_type == ExportType::forwarder)
                    ? m_ptr.forwarderName
                    : nullptr;
            }

            unsigned int ordinal() const noexcept
            {
                return m_ordinal;
            }

            ExportType type() const noexcept
            {
                return m_type;
            }
        };

    private:
        const Pe<arch>& m_pe;
        const typename GenericTypes::ImgDataDir* const m_directory;
        const typename DirExports::Type* const m_descriptor;
        const Tables m_tables;

    public:
        explicit Exports(const Pe<arch>& pe) noexcept
            : m_pe(pe)
            , m_directory(pe.directory(DirExports::k_id))
            , m_descriptor(m_directory ? pe.template byRva<typename DirExports::Type>(m_directory->VirtualAddress) : nullptr)
            , m_tables(m_descriptor
                ? Tables
                {
                    pe.template byRva<typename GenericTypes::ExportAddressTableEntry>(m_descriptor->AddressOfFunctions),
                    pe.template byRva<Rva>(m_descriptor->AddressOfNames),
                    pe.template byRva<Ordinal>(m_descriptor->AddressOfNameOrdinals)
                }
                : Tables{})
        {
        }

        const Pe<arch>& pe() const noexcept
        {
            return m_pe;
        }

        const Rva directoryRva() const noexcept
        {
            return m_directory->VirtualAddress;
        }

        unsigned int directorySize() const noexcept
        {
            return m_directory->Size;
        }

        const Tables& tables() const noexcept
        {
            return m_tables;
        }

        bool contains(const Rva rva) const noexcept
        {
            return (rva >= directoryRva()) && (rva < (directoryRva() + directorySize()));
        }

        const typename DirExports::Type* descriptor() const noexcept
        {
            return m_descriptor;
        }

        bool valid() const noexcept
        {
            return descriptor() != nullptr;
        }

        unsigned int count() const noexcept
        {
            return valid()
                ? descriptor()->NumberOfFunctions
                : 0;
        }

        const char* moduleName() const noexcept
        {
            const Rva rva = descriptor()->Name;
            return m_pe.template byRva<char>(rva);
        }

        unsigned int ordinalBase() const noexcept
        {
            return descriptor()->Base;
        }

        FunctionIterator begin() const noexcept
        {
            return FunctionIterator(*this, 0);
        }

        FunctionIterator end() const noexcept
        {
            return FunctionIterator(*this, count());
        }

        Export find(const char* const funcName) const noexcept
        {
            if (!funcName)
            {
                return {};
            }

            if (!valid())
            {
                return {};
            }

            const auto strByRva = [this](const Rva rva) -> const char*
                {
                    return m_pe.template byRva<char>(rva);
                };

            // [left, right):
            unsigned int left = 0;
            unsigned int right = m_descriptor->NumberOfNames;

            while (left < right)
            {
                unsigned int pos = (left + right) / 2;
                const int cmpRes = strcmp(strByRva(m_tables.namePointerTable[pos]), funcName);
                if (cmpRes > 0)
                {
                    right = pos;
                }
                else if (cmpRes < 0)
                {
                    left = pos + 1;
                }
                else
                {
                    left = pos;
                    break;
                }
            }

            if (left == right)
            {
                return {};
            }

            const Ordinal unbiasedOrdinal = m_tables.nameOrdinalTable[left];
            const auto& exportEntry = m_tables.exportAddressTable[unbiasedOrdinal];
            if (!contains(exportEntry.address))
            {
                return Export(m_pe.template byRva<void>(exportEntry.address), unbiasedOrdinal + ordinalBase(), ExportType::exact);
            }
            else
            {
                return Export(m_pe.template byRva<void>(exportEntry.forwarderString), unbiasedOrdinal + ordinalBase(), ExportType::forwarder);
            }
        }

        Export find(const unsigned int ordinal) const noexcept
        {
            if (!valid())
            {
                return {};
            }

            const unsigned int unbiasedOrdinal = ordinal - ordinalBase();
            if (unbiasedOrdinal >= m_descriptor->NumberOfFunctions)
            {
                return {};
            }

            const auto& exportEntry = m_tables.exportAddressTable[unbiasedOrdinal];
            if (!contains(exportEntry.address))
            {
                return Export(m_pe.template byRva<void>(exportEntry.address), unbiasedOrdinal + ordinalBase(), ExportType::exact);
            }
            else
            {
                return Export(m_pe.template byRva<void>(exportEntry.forwarderString), unbiasedOrdinal + ordinalBase(), ExportType::forwarder);
            }
        }
    };



    template <Arch arch>
    class Relocs
    {
    public:
        class PageEntry;

        class RelocEntry
        {
        private:
            const PageEntry& m_page;
            unsigned int m_index;

        public:
            RelocEntry(const PageEntry& page, const unsigned int index) noexcept : m_page(page), m_index(index)
            {
            }

            const PageEntry& page() const noexcept
            {
                return m_page;
            }

            const Reloc* reloc() const noexcept
            {
                const auto* const relocs = reinterpret_cast<const Reloc*>(m_page.descriptor() + 1);
                return &relocs[m_index];
            }

            const void* addr() const noexcept
            {
                return static_cast<const unsigned char*>(page().page()) + reloc()->offsetInPage;
            }

            bool valid() const noexcept
            {
                return m_index < m_page.count();
            }

            bool operator == (const RelocEntry& entry) const noexcept
            {
                return m_index == entry.m_index;
            }

            RelocEntry& operator ++ () noexcept
            {
                ++m_index;
                return *this;
            }
        };

        using RelocIterator = Iterator<RelocEntry>;

        class PageEntry
        {
        private:
            const Relocs& m_relocs;
            const typename DirRelocs::Type* m_entry;

        public:
            PageEntry(const Relocs& relocs, const typename DirRelocs::Type* entry) noexcept
                : m_relocs(relocs)
                , m_entry(entry)
            {
            }

            bool valid() const noexcept
            {
                return m_entry && m_entry->VirtualAddress && m_entry->SizeOfBlock;
            }

            const typename DirRelocs::Type* descriptor() const noexcept
            {
                return m_entry;
            }

            const void* page() const noexcept
            {
                return m_relocs.pe().template byRva<void>(m_entry->VirtualAddress);
            }

            unsigned int count() const noexcept
            {
                if (!valid())
                {
                    return 0;
                }

                return (m_entry->SizeOfBlock - sizeof(*m_entry)) / sizeof(Reloc); // Without trailing empty element
            }

            bool operator == (const PageEntry& entry) const noexcept
            {
                return descriptor() == entry.descriptor();
            }

            bool operator == (typename Iterator<PageEntry>::TheEnd) const noexcept
            {
                return !valid();
            }

            PageEntry& operator ++ () noexcept
            {
                m_entry = reinterpret_cast<const typename DirRelocs::Type*>(reinterpret_cast<const unsigned char*>(m_entry) + m_entry->SizeOfBlock);
                return *this;
            }

            RelocIterator begin() const noexcept
            {
                return RelocIterator(*this, 0);
            }

            RelocIterator end() const noexcept
            {
                return RelocIterator(*this, count());
            }
        };

        using PageIterator = Iterator<PageEntry>;

    private:
        const Pe<arch>& m_pe;
        const DirectoryDescriptor<DirRelocs> m_descriptor;

    public:
        explicit Relocs(const Pe<arch>& pe) noexcept
            : m_pe(pe)
            , m_descriptor(pe.template directory<DirRelocs>())
        {
        }

        const Pe<arch>& pe() const noexcept
        {
            return m_pe;
        }

        DirectoryDescriptor<DirRelocs> descriptor() const noexcept
        {
            return m_descriptor;
        }

        bool valid() const noexcept
        {
            return m_descriptor.valid();
        }

        PageIterator begin() const noexcept
        {
            return PageIterator(*this, m_descriptor.ptr);
        }

        PageIterator end() const noexcept
        {
            return PageIterator(*this, reinterpret_cast<const typename DirRelocs::Type*>(reinterpret_cast<const unsigned char*>(m_descriptor.ptr) + m_descriptor.size));
        }
    };



    template <Arch arch>
    class Exceptions
    {
    public:
        class RuntimeFunctionEntry
        {
        private:
            const typename DirExceptions::Type* m_runtimeFunction;

        public:
            explicit RuntimeFunctionEntry(const typename DirExceptions::Type* runtimeFunction) noexcept
                : m_runtimeFunction(runtimeFunction)
            {
            }

            const typename DirExceptions::Type* runtimeFunction() const noexcept
            {
                return m_runtimeFunction;
            }

            bool valid() const noexcept
            {
                return m_runtimeFunction && m_runtimeFunction->BeginAddress;
            }

            bool operator == (const RuntimeFunctionEntry& entry) const noexcept
            {
                return runtimeFunction() == entry.runtimeFunction();
            }

            RuntimeFunctionEntry& operator ++ () noexcept
            {
                ++m_runtimeFunction;
                return *this;
            }
        };

        using RuntimeFunctionIterator = Iterator<RuntimeFunctionEntry>;

    private:
        const DirectoryDescriptor<DirExceptions> m_descriptor;

    public:
        explicit Exceptions(const Pe<arch>& pe) noexcept
            : m_descriptor(pe.template directory<DirExceptions>())
        {
        }

        const DirectoryDescriptor<DirExceptions>& descriptor() const noexcept
        {
            return m_descriptor;
        }

        bool valid() const noexcept
        {
            return m_descriptor.valid();
        }

        RuntimeFunctionIterator begin() const noexcept
        {
            return RuntimeFunctionIterator(m_descriptor.ptr);
        }

        RuntimeFunctionIterator end() const noexcept
        {
            return RuntimeFunctionIterator(reinterpret_cast<const typename DirExceptions::Type*>(reinterpret_cast<const unsigned char*>(m_descriptor.ptr) + m_descriptor.size));
        }
    };



    template <Arch arch>
    class Tls
    {
    public:
        class CallbackEntry
        {
        private:
            const Tls& m_tls;
            const typename GenericTypes::FnImageTlsCallback* m_callbackPointer;

        public:
            explicit CallbackEntry(const Tls& tls, const typename GenericTypes::FnImageTlsCallback* const callbacks)
                : m_tls(tls)
                , m_callbackPointer(callbacks)
            {
            }

            typename GenericTypes::FnImageTlsCallback callback() const noexcept
            {
                const Rva rva = static_cast<Rva>(static_cast<unsigned long long>(reinterpret_cast<size_t>(*m_callbackPointer)) - m_tls.pe().imageBase());
                return static_cast<typename GenericTypes::FnImageTlsCallback>(m_tls.pe().template byRva<void>(rva));
            }

            bool operator == (const CallbackEntry& entry) const noexcept
            {
                return m_callbackPointer == entry.m_callbackPointer;
            }

            bool operator == (typename Iterator<CallbackEntry>::TheEnd) const noexcept
            {
                return !m_callbackPointer || !*m_callbackPointer;
            }

            CallbackEntry& operator ++ () noexcept
            {
                ++m_callbackPointer;
                return *this;
            }
        };

        using CallbackIterator = Iterator<CallbackEntry>;

    private:
        const Pe<arch>& m_pe;
        const DirectoryDescriptor<DirTls<arch>> m_directory;

    public:
        explicit Tls(const Pe<arch>& pe) noexcept
            : m_pe(pe)
            , m_directory(pe.template directory<DirTls<arch>>())
        {
        }

        bool valid() const noexcept
        {
            return m_directory.valid();
        }

        const Pe<arch>& pe() const noexcept
        {
            return m_pe;
        }

        const DirectoryDescriptor<DirTls<arch>>& descriptor() const noexcept
        {
            return m_directory;
        }

        const typename GenericTypes::FnImageTlsCallback* callbacks() const noexcept
        {
            if (!valid())
            {
                return nullptr;
            }

            switch (m_pe.type())
            {
            case ImgType::file:
            {
                return m_pe.template byRva<typename GenericTypes::FnImageTlsCallback>(static_cast<Rva>(m_directory.ptr->AddressOfCallBacks - m_pe.imageBase()));
            }
            case ImgType::module:
            {
                return reinterpret_cast<typename GenericTypes::FnImageTlsCallback*>(m_directory.ptr->AddressOfCallBacks);
            }
            }

            return nullptr;
        }

        CallbackIterator begin() const noexcept
        {
            return CallbackIterator(*this, callbacks());
        }

        typename CallbackIterator::TheEnd end() const noexcept
        {
            return {};
        }
    };



    // https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/Object/CVDebugRecord.h
    namespace CodeView
    {

        //
        // PDB signature:
        // 
        //     PDB 2.0: "%s\\%08X%X\\%s",
        //              pdbPath,
        //              signature,
        //              age,
        //              pdbPath
        // 
        //     PDB 7.0: "%s\\%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X%X\\%s",
        //              pdbPath, 
        //              guid.Data1, guid.Data2, guid.Data3,
        //              guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7], guid.Data4[8],
        //              age,
        //              pdbPath
        //

        enum class CodeViewMagic : unsigned int
        {
            pdb70 = 'SDSR', // RSDS
            pdb20 = '01BN', // NB10
        };

        struct DebugInfoPdb20
        {
            CodeViewMagic magic;
            unsigned int offset;
            unsigned int signature;
            unsigned int age;
            char pdbName[1];
        };

        struct DebugInfoPdb70
        {
            CodeViewMagic magic;
            GUID guid;
            unsigned int age;
            char pdbName[1];
        };

        union DebugInfo
        {
            CodeViewMagic magic;
            DebugInfoPdb20 pdb20;
            DebugInfoPdb70 pdb70;
        };

    } // namespace CodeView



    template <Arch arch>
    class Debug
    {
    public:
        class DebugEntry
        {
        private:
            const typename DirDebug::Type* m_debugEntry;

        public:
            explicit DebugEntry(const typename DirDebug::Type* const debugEntry) noexcept : m_debugEntry(debugEntry)
            {
            }

            const typename DirDebug::Type* debugEntry() const noexcept
            {
                return m_debugEntry;
            }

            bool operator == (const DebugEntry& entry) const noexcept
            {
                return m_debugEntry == entry.m_debugEntry;
            }

            DebugEntry& operator ++ () noexcept
            {
                ++m_debugEntry;
                return *this;
            }
        };

        using DebugIterator = Iterator<DebugEntry>;

    private:
        const Pe<arch>& m_pe;
        const DirectoryDescriptor<DirDebug> m_descriptor;

    public:
        explicit Debug(const Pe<arch>& pe) noexcept
            : m_pe(pe)
            , m_descriptor(pe.template directory<DirDebug>())
        {
        }

        const DirectoryDescriptor<DirDebug>& debugTable() const noexcept
        {
            return m_descriptor;
        }

        bool valid() const noexcept
        {
            return m_descriptor.valid();
        }

        unsigned int count() const noexcept
        {
            if (!valid())
            {
                return 0;
            }

            return m_descriptor.size / sizeof(typename DirDebug::Type);
        }

        DebugIterator begin() const noexcept
        {
            return DebugEntry(m_descriptor.ptr);
        }

        DebugIterator end() const noexcept
        {
            return DebugEntry(m_descriptor.ptr + count());
        }

        const CodeView::DebugInfo* findPdbDebugInfo() const noexcept
        {
            for (const auto& entry : *this)
            {
                if (entry.debugEntry()->Type != IMAGE_DEBUG_TYPE_CODEVIEW)
                {
                    continue;
                }

                const auto* const codeView = m_pe.template byRva<CodeView::DebugInfo>(entry.debugEntry()->PointerToRawData);
                switch (codeView->magic)
                {
                case CodeView::CodeViewMagic::pdb20:
                case CodeView::CodeViewMagic::pdb70:
                {
                    return codeView;
                }
                }
            }

            return nullptr;
        }
    };



    template <Arch arch>
    inline Sections Pe<arch>::sections() const noexcept
    {
        const auto* const ntHdr = headers().nt();
        const auto* const firstSection = IMAGE_FIRST_SECTION(ntHdr);
        const auto count = ntHdr->FileHeader.NumberOfSections;
        return Sections(firstSection, count);
    }

    template <Arch arch>
    inline Imports<arch> Pe<arch>::imports() const noexcept
    {
        return Imports<arch>(*this);
    }

    template <Arch arch>
    inline DelayedImports<arch> Pe<arch>::delayedImports() const noexcept
    {
        return DelayedImports<arch>(*this);
    }

    template <Arch arch>
    inline BoundImports<arch> Pe<arch>::boundImports() const noexcept
    {
        return BoundImports<arch>(*this);
    }

    template <Arch arch>
    inline Exports<arch> Pe<arch>::exports() const noexcept
    {
        return Exports<arch>(*this);
    }

    template <Arch arch>
    inline Relocs<arch> Pe<arch>::relocs() const noexcept
    {
        return Relocs<arch>(*this);
    }

    template <Arch arch>
    inline Exceptions<arch> Pe<arch>::exceptions() const noexcept
    {
        return Exceptions<arch>(*this);
    }

    template <Arch arch>
    inline Tls<arch> Pe<arch>::tls() const noexcept
    {
        return Tls<arch>(*this);
    }

    template <Arch arch>
    inline Debug<arch> Pe<arch>::debug() const noexcept
    {
        return Debug<arch>(*this);
    }



} // namespace Pe