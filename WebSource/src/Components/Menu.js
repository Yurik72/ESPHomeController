import React from "react";
import { NavLink } from "react-router-dom";

export default ({ close,services }) => (
    <div className="menu">
        <ul>
            <li>
                <NavLink onClick={close} activeClassName="current" to="/">
                    Home
                 </NavLink>
            </li>
            {services.map(item =>
                <li>
                    <NavLink onClick={close} activeClassName="current" to={"/" + item.name}>
                        {item.name}
                    </NavLink>
                </li>
            )}

            <li>
                <NavLink onClick={close} activeClassName="Services" to="Services">
                    Setup -> Services
                </NavLink>
            </li>
            <li>
                <NavLink onClick={close} activeClassName="Triggers" to="Triggers">
                   Setup ->  Triggers
                </NavLink>
            </li>

        </ul>
    </div>
);
