#include <vp.hpp>

namespace vp::resbui {

	void ByamlBuilder::Serialize(void *output, size_t output_size, ByamlBuilderMemoryInfo *memory_info) {

		/* Clear memory */
		std::memset(output, 0, output_size);

		/* Get regions */
		vp::res::ResByaml *head           = reinterpret_cast<vp::res::ResByaml*>(reinterpret_cast<uintptr_t>(output) + memory_info->offset_header);
		void              *key_table      = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(output) + memory_info->offset_key_table);
		void              *string_pool    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(output) + memory_info->offset_string_pool);
		void              *big_data_base  = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(output) + memory_info->offset_big_data);
		void              *container_base = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(output) + memory_info->offset_container);

		/* Set header info */
        if (!m_is_reverse_endian) {
            head->magic   = vp::res::ResByaml::cMagic;
            head->version = m_version;
        } else {
            head->magic = vp::util::SwapEndian(vp::res::ResByaml::cMagic);
            head->version = vp::util::SwapEndian(m_version);
        }

		/* Nothing else if null root */
		if (m_root_node == nullptr) { return; }

		/* Handle key table */
		if (m_cache.key_table_memory_info.total_size != 0) {
            m_key_table_builder.SetIsReverseEndian(m_is_reverse_endian);
			m_key_table_builder.Serialize(key_table, m_cache.key_table_memory_info.total_size, std::addressof(m_cache.key_table_memory_info));

			/* Set header offset */
            const u32 key_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(key_table) - reinterpret_cast<uintptr_t>(head));
			head->key_table_offset = (!m_is_reverse_endian) ? key_offset : vp::util::SwapEndian(key_offset);
		}

		/* Handle string pool */
		if (m_cache.string_table_memory_info.total_size != 0) {
            m_string_table_builder.SetIsReverseEndian(m_is_reverse_endian);
			m_string_table_builder.Serialize(string_pool, m_cache.string_table_memory_info.total_size, std::addressof(m_cache.string_table_memory_info));

			/* Set header offset */
            const u32 str_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(string_pool) - reinterpret_cast<uintptr_t>(head));
			head->string_table_offset = (!m_is_reverse_endian) ? str_offset : vp::util::SwapEndian(str_offset);
		}

		/* Complete if the container is empty */
		if (memory_info->offset_container == memory_info->GetTotalSize()) { return; }

		/* Check if big data */
		if (ByamlNodeBigData::CheckRuntimeTypeInfoStatic(m_root_node)) {

			/* Serialize big data */
			uintptr_t big_data_iter  = reinterpret_cast<uintptr_t>(big_data_base);
			uintptr_t container_iter = reinterpret_cast<uintptr_t>(container_base);
			reinterpret_cast<ByamlNodeBigData*>(m_root_node)->SerializeForBigData(std::addressof(big_data_iter), std::addressof(container_iter), head);
		}

		/* Set head */
        const u32 data_base = static_cast<u32>(reinterpret_cast<uintptr_t>(container_base) - reinterpret_cast<uintptr_t>(head));
		head->data_offset = (!m_is_reverse_endian) ? data_base : vp::util::SwapEndian(data_base);

		/* Write single data type for non container */
		void *root_value_offset           = container_base;
		res::ByamlDataType root_data_type = m_root_node->GetByamlDataType();
		if (res::IsContainerType(root_data_type) || root_data_type == res::ByamlDataType::Null) { return; }

        u8 *data_type     = reinterpret_cast<u8*>(container_base);
        *data_type        = static_cast<u8>(root_data_type);
        root_value_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(container_base) + sizeof(u32));
		m_root_node->Serialize(root_value_offset, head);

		return;
	}

	void ByamlBuilder::CalculateMemoryInfo(ByamlBuilderMemoryInfo *out_memory_info) {

        if (m_cache.is_cache) {            
            m_key_table_builder.Finalize();
            m_string_table_builder.Finalize();
            if (ByamlNodeBigData::CheckRuntimeTypeInfoStatic(m_root_node)) {
                reinterpret_cast<ByamlNodeBigData*>(m_root_node)->ClearCache();
            }
        }

		/* Set header location */
        uintptr_t iter = 0;
		out_memory_info->offset_header = iter;
        iter += sizeof(vp::res::ResByaml);

		/* Nothing else if null root */
		if (m_root_node == nullptr) {
			out_memory_info->total_size = iter;
			out_memory_info->max_alignment = alignof(u32);
			return;
		}

		/* Gather key table */
		m_root_node->PushKeys(std::addressof(m_key_table_builder));
		m_key_table_builder.CalculateMemoryInfo(std::addressof(m_cache.key_table_memory_info));
        out_memory_info->offset_key_table = iter;
		iter += m_cache.key_table_memory_info.total_size;

		/* Gather string pool */
		m_root_node->PushStrings(std::addressof(m_string_table_builder));
		m_string_table_builder.CalculateMemoryInfo(std::addressof(m_cache.string_table_memory_info));
		out_memory_info->offset_string_pool = iter;
        iter += m_cache.string_table_memory_info.total_size;

        /* Calculate big data and container */
		const size_t big_data_base  = iter;
		uintptr_t    big_data_iter  = big_data_base;
		uintptr_t    container_iter = big_data_base;
		if (ByamlNodeBigData::CheckRuntimeTypeInfoStatic(m_root_node)) {
			reinterpret_cast<ByamlNodeBigData*>(m_root_node)->CalculateBigDataOffsets(std::addressof(big_data_iter), 0);
            container_iter = big_data_iter;
			reinterpret_cast<ByamlNodeBigData*>(m_root_node)->CalculateContainerOffsets(std::addressof(container_iter), 0);
		}
        /* Set big data location */
        out_memory_info->offset_big_data = iter;
        iter += big_data_iter - big_data_base;
        iter = vp::util::AlignUp(iter, alignof(u32));

		/* Set container location */
		out_memory_info->offset_container = iter;
        iter += (container_iter - big_data_iter == 0) ? (sizeof(u32) * 2) : container_iter - big_data_iter;

		/* Set totals */
		out_memory_info->total_size = iter;
		out_memory_info->max_alignment = alignof(u32);

		return;
	}
}
