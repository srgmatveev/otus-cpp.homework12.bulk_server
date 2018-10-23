#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include "bulk.h"
#include "utils.h"
#include "metrics.h"

void BulkReadCmd::process(std::istream &in)
{
    std::string tmp_str;
    while (std::getline(in, tmp_str))
    {
       MetricsCount::Instance().stringsIncr(std::this_thread::get_id());
        this->append(tmp_str);
    }
    if (!open_braces_count && _current_numb_of_cell)
        push();
}

void BulkReadCmd::push()
{
    std::size_t tmp_time = std::chrono::duration_cast<std::chrono::microseconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
    _bulkStorage->set_timestamp(_numb_of_current_chunk, tmp_time);
    notify(*_bulkStorage, _numb_of_current_chunk);
    _current_numb_of_cell = 0;
    _bulkStorage->deleteStorageCell(_numb_of_current_chunk);
}

void BulkReadCmd::append(const std::string &tmp)
{
    if (tmp == "{")
    {
        ++open_braces_count;
        if (open_braces_count == 1)
        {
            if (_current_numb_of_cell > 0)
                push();

            _numb_of_current_chunk = _bulkStorage->create_bulk();
        }
    }
    else if (tmp == "}")
    {

        if (!open_braces_count)
        {
            if (_current_numb_of_cell > 0)
                push();
        }
        else if (!--open_braces_count)
        {
            push();
        }
    }
    else
    {
        if (!open_braces_count)
        {
            if (!_current_numb_of_cell)
                _numb_of_current_chunk = _bulkStorage->create_bulk();

            _bulkStorage->appendToCmdStorage(_numb_of_current_chunk, tmp);

            if (++_current_numb_of_cell == _chunkSize)
                push();
        }
        else
        {
            if (open_braces_count)
            {
                _bulkStorage->appendToCmdStorage(_numb_of_current_chunk, tmp);
            }
        }
    }
}


