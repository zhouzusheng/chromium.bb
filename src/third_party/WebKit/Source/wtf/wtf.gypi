# For GN compatibility, this file can not have any conditions.
{
    'variables': {
        'wtf_files': [
            'ASCIICType.cpp',
            'ASCIICType.h',
            'AddressSanitizer.h',
            'AddressSpaceRandomization.cpp',
            'AddressSpaceRandomization.h',
            'Alias.cpp',
            'Alias.h',
            'Alignment.h',
            'Allocator.h',
            'ArrayBuffer.cpp',
            'ArrayBuffer.h',
            'ArrayBufferBuilder.cpp',
            'ArrayBufferBuilder.h',
            'ArrayBufferContents.cpp',
            'ArrayBufferContents.h',
            'ArrayBufferView.cpp',
            'ArrayBufferView.h',
            'ArrayPiece.cpp',
            'ArrayPiece.h',
            'Assertions.cpp',
            'Assertions.h',
            'Atomics.h',
            'BitArray.h',
            'BitVector.cpp',
            'BitVector.h',
            'BitwiseOperations.h',
            'BloomFilter.h',
            'ByteOrder.h',
            'ByteSwap.h',
            'CPU.h',
            'CheckedArithmetic.h',
            'Compiler.h',
            'ConditionalDestructor.h',
            'ContainerAnnotations.h',
            'CryptographicallyRandomNumber.cpp',
            'CryptographicallyRandomNumber.h',
            'CurrentTime.cpp',
            'CurrentTime.h',
            'DataLog.cpp',
            'DataLog.h',
            'DateMath.cpp',
            'DateMath.h',
            'Deque.h',
            'DoublyLinkedList.h',
            'DynamicAnnotations.cpp',
            'DynamicAnnotations.h',
            'FilePrintStream.cpp',
            'FilePrintStream.h',
            'Float32Array.h',
            'Float64Array.h',
            'Forward.h',
            'Functional.h',
            'GetPtr.h',
            'HashCountedSet.h',
            'HashFunctions.h',
            'HashIterators.h',
            'HashMap.h',
            'HashSet.h',
            'HashTable.cpp',
            'HashTable.h',
            'HashTableDeletedValueType.h',
            'HashTraits.h',
            'HexNumber.h',
            'InstanceCounter.cpp',
            'InstanceCounter.h',
            'Int16Array.h',
            'Int32Array.h',
            'Int8Array.h',
            'IntegralTypedArrayBase.h',
            'LeakAnnotations.h',
            'LinkedStack.h',
            'ListHashSet.h',
            'Locker.h',
            'MainThread.cpp',
            'MainThread.h',
            'MathExtras.h',
            'NonCopyingSort.h',
            'Noncopyable.h',
            'NotFound.h',
            'Optional.h',
            'OwnPtr.h',
            'OwnPtrCommon.h',
            'PageAllocator.cpp',
            'PageAllocator.h',
            'PartitionAlloc.cpp',
            'PartitionAlloc.h',
            'PartitionAllocator.cpp',
            'PartitionAllocator.h',
            'Partitions.cpp',
            'Partitions.h',
            'PassOwnPtr.h',
            'PassRefPtr.h',
            'PassTraits.h',
            'PrintStream.cpp',
            'PrintStream.h',
            'RefCounted.h',
            'RefCountedLeakCounter.cpp',
            'RefCountedLeakCounter.h',
            'RefPtr.h',
            'RetainPtr.h',
            'SaturatedArithmetic.h',
            'SizeLimits.cpp',
            'SpinLock.cpp',
            'SpinLock.h',
            'StaticConstructors.h',
            'StdLibExtras.h',
            'StringExtras.h',
            'StringHasher.h',
            'TemporaryChange.h',
            'ThreadRestrictionVerifier.h',
            'ThreadSafeRefCounted.h',
            'ThreadSpecific.h',
            'ThreadSpecificWin.cpp',
            'Threading.h',
            'ThreadingPrimitives.h',
            'ThreadingPthreads.cpp',
            'ThreadingWin.cpp',
            'TreeNode.h',
            'TypeTraits.cpp',
            'TypeTraits.h',
            'TypedArrayBase.h',
            'Uint16Array.h',
            'Uint32Array.h',
            'Uint8Array.h',
            'Utility.h',
            'Vector.h',
            'VectorTraits.h',
            'WTF.cpp',
            'WTF.h',
            'WTFExport.h',
            'WTFThreadData.cpp',
            'WTFThreadData.h',
            'WeakPtr.h',
            'asm/SaturatedArithmeticARM.h',
            'dtoa.cpp',
            'dtoa.h',
            'dtoa/bignum-dtoa.cc',
            'dtoa/bignum-dtoa.h',
            'dtoa/bignum.cc',
            'dtoa/bignum.h',
            'dtoa/cached-powers.cc',
            'dtoa/cached-powers.h',
            'dtoa/diy-fp.cc',
            'dtoa/diy-fp.h',
            'dtoa/double-conversion.cc',
            'dtoa/double-conversion.h',
            'dtoa/double.h',
            'dtoa/fast-dtoa.cc',
            'dtoa/fast-dtoa.h',
            'dtoa/fixed-dtoa.cc',
            'dtoa/fixed-dtoa.h',
            'dtoa/strtod.cc',
            'dtoa/strtod.h',
            'dtoa/utils.h',
            'text/ASCIIFastPath.h',
            'text/AtomicString.cpp',
            'text/AtomicString.h',
            'text/AtomicStringCF.cpp',
            'text/AtomicStringHash.h',
            'text/Base64.cpp',
            'text/Base64.h',
            'text/CString.cpp',
            'text/CString.h',
            'text/CharacterNames.h',
            'text/Collator.h',
            'text/IntegerToStringConversion.h',
            'text/StringBuffer.h',
            'text/StringBuilder.cpp',
            'text/StringBuilder.h',
            'text/StringConcatenate.cpp',
            'text/StringConcatenate.h',
            'text/StringHash.h',
            'text/StringImpl.cpp',
            'text/StringImpl.h',
            'text/StringImplCF.cpp',
            'text/StringImplMac.mm',
            'text/StringMac.mm',
            'text/StringOperators.h',
            'text/StringStatics.cpp',
            'text/StringUTF8Adaptor.h',
            'text/StringView.h',
            'text/TextCodec.cpp',
            'text/TextCodecASCIIFastPath.h',
            'text/TextCodecICU.cpp',
            'text/TextCodecLatin1.cpp',
            'text/TextCodecReplacement.cpp',
            'text/TextCodecReplacement.h',
            'text/TextCodecUTF16.cpp',
            'text/TextCodecUTF8.cpp',
            'text/TextCodecUTF8.h',
            'text/TextCodecUserDefined.cpp',
            'text/TextEncoding.cpp',
            'text/TextEncodingRegistry.cpp',
            'text/TextPosition.cpp',
            'text/TextPosition.h',
            'text/UTF8.cpp',
            'text/UTF8.h',
            'text/Unicode.h',
            'text/WTFString.cpp',
            'text/WTFString.h',
            'text/icu/CollatorICU.cpp',
            'text/icu/UnicodeIcu.h',
        ],
        'wtf_unittest_files': [
            'ASCIICTypeTest.cpp',
            'ArrayBufferBuilderTest.cpp',
            'AssertionsTest.cpp',
            'CheckedArithmeticTest.cpp',
            'DequeTest.cpp',
            'FunctionalTest.cpp',
            'HashMapTest.cpp',
            'HashSetTest.cpp',
            'ListHashSetTest.cpp',
            'MathExtrasTest.cpp',
            'OptionalTest.cpp',
            'PartitionAllocTest.cpp',
            'RefPtrTest.cpp',
            'SaturatedArithmeticTest.cpp',
            'StringExtrasTest.cpp',
            'StringHasherTest.cpp',
            'TemporaryChangeTest.cpp',
            'TreeNodeTest.cpp',
            'VectorTest.cpp',
            'dtoa_test.cpp',
            'testing/WTFTestPrintersTest.cpp',
            'text/AtomicStringTest.cpp',
            'text/CStringTest.cpp',
            'text/StringBufferTest.cpp',
            'text/StringBuilderTest.cpp',
            'text/StringImplTest.cpp',
            'text/StringOperatorsTest.cpp',
            'text/TextCodecReplacementTest.cpp',
            'text/TextCodecUTF8Test.cpp',
            'text/WTFStringTest.cpp',
        ],
        'wtf_unittest_helper_files': [
            'testing/WTFTestPrinters.cpp',
        ],
    },
}
