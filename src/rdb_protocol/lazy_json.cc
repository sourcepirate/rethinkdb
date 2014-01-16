// Copyright 2010-2014 RethinkDB, all rights reserved.
#include "rdb_protocol/lazy_json.hpp"

#include "containers/archive/buffer_group_stream.hpp"
#include "rdb_protocol/blob_wrapper.hpp"

using namespace alt;  // RSI

counted_t<const ql::datum_t> get_data(const rdb_value_t *value,
                                      alt_buf_parent_t parent) {
    rdb_blob_wrapper_t blob(parent.cache()->get_block_size(),
                            const_cast<rdb_value_t *>(value)->value_ref(),
                            alt::blob::btree_maxreflen);

    counted_t<const ql::datum_t> data;

    alt::blob_acq_t acq_group;
    buffer_group_t buffer_group;
    blob.expose_all(parent, alt_access_t::read, &buffer_group, &acq_group);
    buffer_group_read_stream_t read_stream(const_view(&buffer_group));
    archive_result_t res = deserialize(&read_stream, &data);
    guarantee_deserialization(res, "rdb value");

    return data;
}

const counted_t<const ql::datum_t> &lazy_json_t::get() const {
    guarantee(pointee.has());
    if (!pointee->ptr.has()) {
        pointee->ptr = get_data(pointee->rdb_value, pointee->parent);
        pointee->rdb_value = NULL;
        pointee->parent = alt_buf_parent_t();
    }
    return pointee->ptr;
}

void lazy_json_t::reset() {
    pointee.reset();
}
