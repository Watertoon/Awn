#pragma once

namespace awn::para {

    struct CombinationDataCellBase_Layout {
        const char *RowKey    = "";
        const char *ColumnKey = "";

        AWN_PARA_LAYOUT_TYPE_TRAITS(CombinationDataCellBase_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__CombinationDataCellBase, CombinationDataCellBase,
        TypedParam,                         CombinationDataCellBase_Layout,
        RowKey,
        ColumnKey
    );

	struct CombinationDataTableData_Layout {
		TypedParamMap<CombinationDataCellBase>  CellList  = {};
        const char                             *TableType = "";

        AWN_PARA_LAYOUT_TYPE_TRAITS(CombinationDataTableData_Layout);
	};
    
    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__CombinationDataTableData, CombinationDataTableData,
        TypedParam,                          CombinationDataTableData_Layout,
        TableType,
        CellList
    );

    enum class CellArrangement : bool {
        Row    = 0,
        Column = 1,
    };

    class CellIterator {
        public:
            using TreeIter = TypedParamMap<CombinationDataCellBase>::TypedParamTree::TreeType::const_iterator;
        private:
            TreeIter    m_iter;
            TreeIter    m_end_iter;
            const char *m_key;
            bool        m_is_col_iter;
        public:
            constexpr CellIterator() : m_iter(nullptr), m_end_iter(nullptr) {/*...*/}
            CellIterator(const TypedParamMap<CombinationDataCellBase> *map, CellArrangement arrangement);
            CellIterator(const CellIterator &iter, CellArrangement arrangement);
            constexpr ~CellIterator() {/*...*/}

            constexpr bool IsValid() const {
                return m_iter != m_end_iter;
            }

            constexpr const char *GetKey() const {
                return m_key;
            }

            const char *GetArrangementKey() const;
            const char *GetRowKey() const;
            const char *GetColumnKey() const;

            void SetArrangement(CellArrangement arrangment);

            template <typename T>
                requires (TypedParam::CheckRuntimeTypeInfoStatic(T::GetRuntimeTypeInfoStatic()))
            T *GetTypedParam() {
                para::TypedParam *typed_param = (*m_iter).typed_param;
                T *result = (T::CheckRuntimeTypeInfoStatic(typed_param)) ? reinterpret_cast<T*>(typed_param) : nullptr;
                return result;
            }

            const TypedParamMapNode *operator*();

            CellIterator &operator++();

            constexpr bool operator==(const CellIterator &rhs) const {
                return m_iter == rhs.m_iter;
            }
    };

    ALWAYS_INLINE CellIterator BeginRow(CombinationDataTableData *data_table) {
        const TypedParamMap<CombinationDataCellBase> *map = data_table->GetCellList();
        return CellIterator(map, CellArrangement::Row);
    }
    ALWAYS_INLINE CellIterator BeginColumn(CombinationDataTableData *data_table) {
        const TypedParamMap<CombinationDataCellBase> *map = data_table->GetCellList();
        return CellIterator(map, CellArrangement::Column);
    }

    ALWAYS_INLINE CellIterator GetRowIteratorTo(const CellIterator &iter) {
        return CellIterator(iter, CellArrangement::Row);
    }
    ALWAYS_INLINE CellIterator GetColumnIteratorTo(const CellIterator &iter) {
        return CellIterator(iter, CellArrangement::Column);
    }

    constexpr CellIterator EndRow() {
        return CellIterator();
    }
    constexpr CellIterator EndColumn() {
        return CellIterator();
    }

    class RowIterator {
        private:
            CombinationDataTableData *m_data_table;
        public:
            constexpr ALWAYS_INLINE RowIterator(CombinationDataTableData *table) : m_data_table(table) {/*...*/}
            constexpr ALWAYS_INLINE ~RowIterator() {/*...*/}

            ALWAYS_INLINE auto begin() {
                return BeginRow(m_data_table);
            }
            constexpr ALWAYS_INLINE auto end() {
                return EndRow();
            }
    };
    class ColumnIterator {
        private:
            CombinationDataTableData *m_data_table;
        public:
            constexpr ALWAYS_INLINE ColumnIterator(CombinationDataTableData *table) : m_data_table(table) {/*...*/}
            constexpr ALWAYS_INLINE ~ColumnIterator() {/*...*/}

            ALWAYS_INLINE auto begin() {
                return BeginColumn(m_data_table);
            }
            constexpr ALWAYS_INLINE auto end() {
                return EndColumn();
            }
    };

    u32 GetRowCount(CombinationDataTableData *data_table);
    u32 GetColumnCount(CombinationDataTableData *data_table);
}
