/*
Copyright (c) 2025 acrion innovations GmbH
Authors: Stefan Zipproth, s.zipproth@acrion.ch

This file is part of Cbeam, see https://github.com/acrion/cbeam and https://cbeam.org

Cbeam is offered under a commercial and under the AGPL license.
For commercial licensing, contact us at https://acrion.ch/sales. For AGPL licensing, see below.

AGPL licensing:

Cbeam is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Cbeam is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with Cbeam. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

namespace cbeam::concurrency
{
    template <typename Derived>
    class threaded_object;

    template <typename Message>
    class message_manager;
}

namespace cbeam::logging
{
    class logger
    {
    public:
        logger(const std::filesystem::path& log_file_path, const std::size_t queue_size = 1);

    private:
        std::string create_json_log_entry(
            const std::string& time_stamp,
            const std::string& thread_id,
            const std::string& thread_name,
            const int          log_level);

        std::unique_ptr<concurrency::message_manager<message>> _message_manager;
    };
}
